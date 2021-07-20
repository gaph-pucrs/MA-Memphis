#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>


#define FIXE 4 /*nb de chiffres après la virgule*/
#define MAX 1000000000 /*10^PU*/
#define PU 9 /*puissance de 10 max supportée ici 2³¹->2 000 000 000 donc 10⁹*/
#define size 16
#define data 64
#define data_val 640000 /*valeur de data et size avec FIXE chiffres après la virgule*/
#define size_val 160000
#define region 1
#define add(a, b) (a + b)
#define sub(a, b) (a - b)




int mult(int a, int b)
{
	int i,res;
	int cpt1=0;  /*détermine l'ordre de grandeur en puissance de 10 de a*/
	int cpt2=0;  /*détermine l'ordre de grandeur en puissance de 10 de b*/
	int cpt =2*FIXE; /*compte le nombre de décimales*/
	int a2,b2;

	a2=a;
	b2=b;

	/*détermine l'ordre de grandeur en puissance de 10 de a*/
	while(a2!=0)
	{
		a2=a2/10;
		cpt1++;
	}
	cpt1--;

	/*détermine l'ordre de grandeur en puissance de 10 de b*/
	while(b2!=0)
	{
		b2=b2/10;
		cpt2++;
	}
	cpt2--;

	/*divise a et b en conséquence lorsque ceux-ci sont trop grand*/
	/*on sacrifie en précision pour pouvoir effectuer le calcul*/
	while((cpt1+cpt2)>=PU-1)
	{
		a=a/10;
		cpt1--;
		cpt--;
		if((cpt1+cpt2)>=PU-1)
		{
			b=b/10;
			cpt2--;
			cpt--;
		}
	}

	/*calcul de la multiplication*/
	res=a*b;

	/*retire les derniers chiffres après la virgule pour n'en garder que FIXE*/
	if(cpt>FIXE)
	{
		for(i=0;i<(cpt-FIXE);i++)
		{
			res=res/10;
		}
	}
	else
	{
		while(cpt<FIXE)
		{
			res=res*10;
			cpt++;
		}
	}
	return res;
}


int sommeXYZ(int* moyenne,int* tabref)
{
        int i;
        int sum=0;
        for(i=0;i<size;i++)
        {
               sum=add(mult(tabref[i],moyenne[i]),sum);

        }
        return sum;
}


message_t msg1,msg2;

/*int tabdata[data]={490000,490000,489990,489990,489980,489980,489970,489970,
					489960,489960,489950,489950,489940,489940,489930,489930};*/



int tabrefX[size]={1,2,2,3,5,6,8,11,14,18,23,29,37,47,58,72};
int tabrefY[size]={0,0,0,0,0,0,0,1,1,2,3,3,4,5,6,8};
int tabrefZ[size]={7,9,12,17,22,29,38,50,64,82,105,133,167,210,261,323};




int main()
{
	int i;
	puts("start XYZ1 1\n");
	//printf("%d\n", memphis_get_tick());
	

	memphis_receive(&msg1,P1);


    msg2.length=3;

    msg2.payload[0]=sommeXYZ((int *)msg1.payload,tabrefX);
    msg2.payload[1]=sommeXYZ((int *)msg1.payload,tabrefY);
    msg2.payload[2]=sommeXYZ((int *)msg1.payload,tabrefZ);
    puts("XYZ :\n");
    for(i=0;i<3;i++)
         //printf("%d\n", fixetoa(msg2.payload[i]));

    memphis_send(&msg2,LAB1);
    memphis_send(&msg2,DXYZ);
    memphis_send(&msg2,RGB1);




	//printf("%d\n", memphis_get_tick());
	puts("Communication XYZ1 finished.\n");

return 0;
}
