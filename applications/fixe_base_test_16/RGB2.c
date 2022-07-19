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




message_t msg1,msg2;


void rgb(int* sum,int* RGB)
{

    RGB[0]=add(sub(mult(23706,sum[0]),mult(5138,sum[1])),mult(53,sum[2]));
     RGB[1]=sub(sub(mult(14253,sum[1]),mult(9000,sum[0])),mult(147,sum[2]));
    RGB[2]=add(sub(mult(886,sum[1]),mult(4706,sum[0])),mult(10094,sum[2]));
}



int main()
{
	puts("start RGB2\n");

    int RGB[3],i;

	memphis_receive(&msg1,XYZ2);

	rgb((int*)msg1.payload,RGB);

    msg2.length=3;
    for(i=0;i<3;i++)
         msg2.payload[i]=RGB[i];

    // Echo("Valeur de RGB :");
    // for(i=0;i<3;i++)
    // {
	// 	Echo(fixetoa(RGB[i]));
	// }

    memphis_send(&msg2,DRGB);

	puts("Communication RGB2 finished.\n");

	return 0;
}

