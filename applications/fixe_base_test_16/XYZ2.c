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
#define add(a, b) ((int)a + (int)b)
#define sub(a, b) ((int)a - (int)b)

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

	/*div_fixedise a et b en conséquence lorsque ceux-ci sont trop grand*/
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




int msg1[size], msg2[3];


/*int tabdata[data]={490000,490000,489990,489990,489980,489980,489970,489970
					,800000,489960,489950,900000,489940,489940,489930,489930};*/


int tabdata[data]={490000,490000,489990,489990,489980,489980,489970,489970
					,800000,489960,489950,900000,489940,489940,489930,489930,
					489920,489920,489910,489910,1100000,489900,489890,489890,
					489880,489880,489870,489870,1650000,489860,489850,489850,
					489840,489840,880000,489830,489820,489820,489810,489810,
					489800,1000000,489790,489790,489790,489780,489780,489770,
					489770,489760,489760,489750,489750,489740,489740,489730,
					489730,489720,489720,489710,489710,489700,489700,489690};


int tabrefX[size]={1,2,2,3,5,6,8,11,14,18,23,29,37,47,58,72};
int tabrefY[size]={0,0,0,0,0,0,0,1,1,2,3,3,4,5,6,8};
int tabrefZ[size]={7,9,12,17,22,29,38,50,64,82,105,133,167,210,261,323};


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


int main()
{
	// int i;
	puts("start XYZ 2\n");

    memphis_receive(msg1, sizeof(msg1), P2);

	printf("Received MSG1\n");
    msg2[0]=sommeXYZ(msg1, tabrefX);
    msg2[1]=sommeXYZ(msg1, tabrefY);
    msg2[2]=sommeXYZ(msg1, tabrefZ);
    // Echo("XYZ :");
    // for(i=0;i<3;i++)
    //      Echo(fixetoa(msg2.payload[i]));

    memphis_send(msg2, sizeof(msg2), LAB2);
    memphis_send(msg2, sizeof(msg2), DXYZ);
    memphis_send(msg2, sizeof(msg2), RGB2);

    puts("Communication XYZ 2 finished.\n");

	return 0;
}

