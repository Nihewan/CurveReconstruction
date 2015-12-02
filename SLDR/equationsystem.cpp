#include "stdafx.h"
#include "mathlib.h"
#include "CP_PointVector.h"
#include <math.h>

namespace math_lib {

	//高斯消元法中选主元素的程序
	void guassSelect(double **&a,int *&z,int k,int np)
	{
		int xi = 0, yj = 0;
		double tmp = 0;
		for( int i = k; i < np; i++)
		{
			for(int j=k; j < np; j++)
			{
				if(fabs(a[i][j])>fabs(tmp))
				{
					tmp=a[i][j];
					xi=i;
					yj=j;
				}
			}
		}
		if(xi != k)
		{
			for( int i=k; i<np+1; i++)
			{
				tmp=a[k][i];
				a[k][i]=a[xi][i];
				a[xi][i]=tmp;
			}
		}
		if(yj!=k)
		{
			for(int i=0; i<np; i++)
			{
				tmp=a[i][k];
				a[i][k]=a[i][yj];
				a[i][yj]=tmp;
			}

			int temp=z[k];
			z[k]=z[yj];
			z[yj]=temp;
		}
	}


	//高斯消元法解方程
	int EquatSystemPoint(int np, double **aa, CP_Point3D *bb, CP_Point3D *x)
	{
		int i, j, k;
		double **a;
		int *z;
		double *xx;
		z=new int[np];
		xx=new double[np];

		a = new double *[np];
		for ( i = 0; i < np; i++ )
			a[i] = new double [np+1];

		//计算x值
		for(i=0; i<np; i++) 
			z[i]=i;

		for ( i = 0; i < np; i++ )
			for ( j = 0; j < np; j++ )
				a[i][j] = aa[i][j];
		for ( i = 0; i < np; i++ )
			a[i][np] = bb[i].m_x;

		for( k=0; k < np; k++ )
		{
			guassSelect(a,z,k,np);

			for(i=k+1;i < np; i++)
			{
				double l=a[i][k]/a[k][k];
				for(j=k+1; j<=np; j++)
					a[i][j] -= l*a[k][j];
			}
		}

		for ( i = 0; i < np; i++ )
			xx[i] = 0;

		if( fabs(a[np-1][np-1]) < TOLER )
		{// Should free memory before return. By Jun-Hai Yong: 2002-12-7
			for ( i = 0; i < np; i++ )
				delete [] a[i];
			delete []a;
			delete []xx;
			delete []z;
			return 0;
		}

		for(i=np-1; i >= 0 ;i--)
		{
			for( j = i+1;j < np; j++)
				a[i][np] -= a[i][j]*xx[j];

			xx[i]=a[i][np]/a[i][i];
		}
		for(i=0; i < np; i++)
		{
			x[z[i]].m_x=xx[i];
		}

		//计算y值
		for(i=0; i<np; i++) 
			z[i]=i;

		for ( i = 0; i < np; i++ )
			for ( j = 0; j < np; j++ )
				a[i][j] = aa[i][j];
		for ( i = 0; i < np; i++ )
			a[i][np] = bb[i].m_y;

		for( k=0; k < np; k++ )
		{
			guassSelect(a,z,k,np);

			for(i=k+1;i < np; i++)
			{
				double l=a[i][k]/a[k][k];
				for(j=k+1; j<=np; j++)
					a[i][j] -= l*a[k][j];
			}
		}

		for ( i = 0; i < np; i++ )
			xx[i] = 0;

		if( fabs(a[np-1][np-1]) < TOLER )
		{// Should free memory before return. By Jun-Hai Yong: 2002-12-7
			for ( i = 0; i < np; i++ )
				delete [] a[i];
			delete []a;
			delete []xx;
			delete []z;
			return 0;
		}

		for(i=np-1; i >= 0 ;i--)
		{
			for( j = i+1;j < np; j++)
				a[i][np] -= a[i][j]*xx[j];

			xx[i]=a[i][np]/a[i][i];
		}
		for(i=0; i < np; i++)
		{
			x[z[i]].m_y=xx[i];
		}

		//计算z值
		for(i=0; i<np; i++) 
			z[i]=i;

		for ( i = 0; i < np; i++ )
			for ( j = 0; j < np; j++ )
				a[i][j] = aa[i][j];
		for ( i = 0; i < np; i++ )
			a[i][np] = bb[i].m_z;

		for( k=0; k < np; k++ )
		{
			guassSelect(a,z,k,np);

			for(i=k+1;i < np; i++)
			{
				double l=a[i][k]/a[k][k];
				for(j=k+1; j<=np; j++)
					a[i][j] -= l*a[k][j];
			}
		}

		for ( i = 0; i < np; i++ )
			xx[i] = 0;

		if( fabs(a[np-1][np-1]) < TOLER )
		{// Should free memory before return. By Jun-Hai Yong: 2002-12-7
			for ( i = 0; i < np; i++ )
				delete [] a[i];
			delete []a;
			delete []xx;
			delete []z;
			return 0;
		}

		for(i=np-1; i >= 0 ;i--)
		{
			for( j = i+1;j < np; j++)
				a[i][np] -= a[i][j]*xx[j];

			xx[i]=a[i][np]/a[i][i];
		}
		for(i=0; i < np; i++)
		{
			x[z[i]].m_z=xx[i];
		}

		for ( i = 0; i < np; i++ )
			delete [] a[i];
		delete []a;
		delete []xx;
		delete []z;
		return 1;
	}

	//高斯消元法解方程
	int EquatSystemNumber(int np, double **aa, double *bb, double *x)
	{
		int i, j, k;
		double **a;
		int *z;
		double *xx;
		z=new int[np];
		xx=new double[np];

		a = new double *[np];
		for ( i = 0; i < np; i++ )
			a[i] = new double [np+1];

		for(i=0; i<np; i++) 
			z[i]=i;

		for ( i = 0; i < np; i++ )
			for ( j = 0; j < np; j++ )
				a[i][j] = aa[i][j];
		for ( i = 0; i < np; i++ )
			a[i][np] = bb[i];

		for( k=0; k < np; k++ )
		{
			guassSelect(a,z,k,np);

			for(i=k+1;i < np; i++)
			{
				double l=a[i][k]/a[k][k];
				for(j=k+1; j<=np; j++)
					a[i][j] -= l*a[k][j];
			}
		}

		for ( i = 0; i < np; i++ )
			xx[i] = 0;

		if( fabs(a[np-1][np-1]) < TOLER )
		{// Should free memory before return. By Jun-Hai Yong: 2002-12-7
			for ( i = 0; i < np; i++ )
				delete [] a[i];
			delete []a;
			delete []xx;
			delete []z;
			return 0;
		}

		for(i=np-1; i >= 0 ;i--)
		{
			for( j = i+1;j < np; j++)
				a[i][np] -= a[i][j]*xx[j];

			xx[i]=a[i][np]/a[i][i];
		}
		for(i=0; i < np; i++)
		{
			x[z[i]]=xx[i];
		}

		for ( i = 0; i < np; i++ )
			delete [] a[i];
		delete []a;
		delete []xx;
		delete []z;
		return 1;
	}
}