# Adding new management tasks

Management tasks are different from user tasks.
They are injected by the MAInjector and their folder structure is different.
Inside the [management folder](/management), each subfolder is a task.
Inside each task subfolder there is a `config.yaml` file that indicates the capabilities of the management task.
Currently, the following capabilities are supported:

* act
  * migration
* decide
  * qos
* observe
  * qos

Inside the `src` subfolder of each task, the `.c` files are included, and inside the `include` subfolder of the `src` folder, the header `.h` files are included.
To add a new management task, simply add a new task in the same file and folder structure as described.

## Adding new capabilities

To add new capabilities, edit the [ma_builder](../build_env/scripts/ma_builder.py) and add to the `get_task_type_tag` function the desired tag with a exclusive bit indicating in the following format:

LSB indicates the type of the task (Actuation, Decision, Observation or User task):
| 7 | 6 | 5 | 4 | 3       | 2      | 1   | 0    |
|---|---|---|---|---------|--------|-----|------|
|   |   |   |   | Observe | Decide | Act | User |

The next byte indicates the Observation capabilities:
| 15 | 14 | 13 | 12 | 11 | 10 | 9 | 8   |
|----|----|----|----|----|----|---|-----|
|    |    |    |    |    |    |   | QoS |

The next byte indicates the Decision capabilities:
| 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16  |
|----|----|----|----|----|----|----|-----|
|    |    |    |    |    |    |    | QoS |

And finally the MSB indicates the Actuation capabilities:
| 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24        |
|----|----|----|----|----|----|----|-----------|
|    |    |    |    |    |    |    | Migration |

Empty spaces in the tables above indicates that there is no defined capability yet.
