import random
import math

def write_file(name, extension, value):
    with open(f"{name}.{extension}","w+") as f:
        f.writelines(value)

def write_config_yaml(tasks):
    config_yaml = f"""ctg:
  master:
    successors:\n"""
    for i in range(1, tasks):
        if i < 10:
            config_yaml += f"      - \"worker0{i}\"\n"
        else:
            config_yaml += f"      - \"worker{i}\"\n"
        
    # print(config_yaml)
    return config_yaml

def write_h(vector_size, tasks, order_by):
    h = f"""#ifndef MAP_REDUCE_STD_H_
#define MAP_REDUCE_STD_H_

#define NUMBER_OF_TASKS {tasks}
#define ARRAY_LEN {vector_size}

int arr[{vector_size}] = {{\n"""

    # random.seed(42)

    arr = []
    sum = 0
    for i in range(vector_size):
        if order_by == 'rand' or order_by == 'random':
            aux = random.randrange(0, 65000)
        else:
            aux = i
        arr.append(aux)
        sum += aux

    if order_by == 'asc' or order_by == 'ascending':
        pass
    elif order_by == 'desc' or order_by == 'descending':
        arr = arr[::-1]

    
    for i in range(0, vector_size, 10):
        slice = arr[i:i+10]
        if i < int(vector_size - 10):
            slice_str = ', '.join(str(x) for x in slice) + ",\n"
            # print(slice_str)
            h += slice_str
        else:
            slice_str = ', '.join(str(x) for x in slice) + "\n"
            # print(slice_str)
            h += slice_str

    h_end = f"""}};

int sum(unsigned int arr[], int n)
{{
    unsigned int sum = 0;
  
    for (int i = 0; i < n; i++)
    sum += arr[i];
  
    return sum;
}};

#endif"""

    h += h_end
    # print(h)
    # print(2_147_483_647)
    return h, sum

def write_master(tasks):
    master = f"""#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "map_reduce_std.h"

message_t msg;
message_t msg1;

int main()
{{
    int div_number = ARRAY_LEN/NUMBER_OF_TASKS;
    unsigned int sum_cum = 0;
    int values_to_sent = (ARRAY_LEN/NUMBER_OF_TASKS)*NUMBER_OF_TASKS;
    int missing_values = ARRAY_LEN-values_to_sent;
    int values_to_stay = missing_values+div_number;
    unsigned int arr_malloc[values_to_stay];

    printf("Fatiando e enviando...\\n");

    for(int j=0; j<values_to_sent; j+=div_number) {{
        if((j/div_number)+1 < NUMBER_OF_TASKS) {{
            __builtin_memcpy(msg.payload, arr + j, div_number*sizeof(int));
            msg.length = div_number;
            memphis_send(&msg, (j/div_number)+1);
        }}
        else {{
            __builtin_memcpy(arr_malloc, arr + j, values_to_stay*sizeof(int));
        }}
    }}

    printf("Recebendo...\\n");

    sum_cum = sum(arr_malloc, values_to_stay);

"""

    for i in range(int(math.log2(tasks))):
        if 2**i < 10:
            master += f"""    memphis_receive(&msg1, worker0{2**i});
    sum_cum += sum(msg1.payload, div_number);\n"""
        else:
            master += f"""    memphis_receive(&msg1, worker{2**i});
    sum_cum += sum(msg1.payload, div_number);\n"""

    master_end = f"""\n    printf("Soma acumulada:  %d\\n", sum_cum);

    return 0;
}}"""

    master += master_end
    
    return master

def write_workers(id_, tasks):
    worker = f"""#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "map_reduce_std.h"

message_t msg;
message_t msg1;

int main()
{{
    unsigned int sum_cum = 0;

    printf("Worker%d\\n", memphis_get_id()-255-1);

    memphis_receive(&msg, master);

    printf("Recebeu\\n");
	
    sum_cum += sum(msg.payload, msg.length);
\n"""
    
    if id_%2 != 0:
        worker += f"""    msg1.payload[0]=sum_cum;
    msg1.length = 1;\n"""
        if id_ -1 == 0:
            target = "master"
        else:
            if id_ < 10:
                target = f"worker0{id_-1}"
            else:
                target = f"worker{id_-1}"
        worker += f"""    memphis_send(&msg1, {target});
    
    printf("Enviou\\n");

    return 0;
}}"""
    else:
        to_receive, to_send = get_targets(id_, tasks)
        for i in to_receive:
            if i < 10:
                worker += f"""    memphis_receive(&msg1, worker0{i});
        sum_cum += sum(msg1.payload, msg.length);\n"""
            else:
                worker += f"""    memphis_receive(&msg1, worker{i});
    sum_cum += sum(msg1.payload, msg.length);\n"""

        worker += f"""\n    msg1.payload[0]=sum_cum;
    msg1.length = 1;\n"""

        if to_send == 0:
            target = "master"
        else:
            if to_send < 10:
                target = f"worker0{to_send}"
            else:
                target = f"worker{to_send}"
        worker += f"""    memphis_send(&msg1, {target});
    
    printf("Enviou\\n");

    return 0;
}}"""
        # print(worker)
    
    return worker
    # print(worker)

def get_targets(id_, tasks):
    arr_idx = []
    arr_dest = []
    dict_receive = {}
    dict_send = {}

    for i in range(tasks):
        arr_idx.append(i)
        arr_dest.append([])

    xx = int(math.log2(tasks))

    for i in range(xx):
        if i == 0:
            for j in arr_idx:
                if j%2 == 0:
                    arr_dest[j].append(arr_idx[j+1])    
            remove_indices = []
            for j in arr_idx:
                if len(arr_dest[j]) == 0:
                    remove_indices.append(j)
            arr_idx = [i for j, i in enumerate(arr_idx) if j not in remove_indices]
            arr_dest = [i for j, i in enumerate(arr_dest) if j not in remove_indices]
        else:
            remove_indices = []
            for jj in range(0, len(arr_idx), 2):
                arr_dest[jj].append(arr_idx[jj+1])
                remove_indices.append(jj+1)
            arr_idx = [i for j, i in enumerate(arr_idx) if j not in remove_indices]
            arr_dest = [i for j, i in enumerate(arr_dest) if j not in remove_indices]
        
        for idx, row in enumerate(arr_idx):
            dict_receive[row] = arr_dest[idx]
            dict_send[arr_dest[idx][-1]] = row
    
    return dict_receive[id_], dict_send[id_]

if __name__ == '__main__':
    print(f"Utils")
