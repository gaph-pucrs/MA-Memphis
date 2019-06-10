Iaçanã Ianiski Weber - 06/2019
-- Sorting by Divide and Conquer --
In this application the Master task sends half vector to each of its branchs, this process
is repeated more two times in the interns layers. Once the vector reachs the leaf task, 
a Quick Sort algorithm is applied and the resultant organized vector returns to the upper tasks
that will merge both organized vectors from the leafs and return all the way up to the master task.

                             (MASTER)
                    (1)                     (2)
                (3)     (4)             (5)     (6)
              (7) (8) (9)(10)        (11)(12) (13)(14)