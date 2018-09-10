#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include<mpi.h>
using namespace std;

unsigned t0, t1;

struct Estacion {
  int id;
  int linea;
  string cod;
  string nombre;
  int comb;
};

void Llenado(vector <Estacion> &metro)
{
	Estacion aux;
	string l,auxStr,auxStr2;
	int posicion,contador=1,lineaAux;
	ifstream fixero("Lineas.csv");
	while(!fixero.eof())
	{
		getline(fixero,l);
		posicion=l.find(" ");
		auxStr = l.substr(0,posicion);
		posicion=l.find(" ")+1;
		auxStr2 =l.substr(posicion);
		posicion=auxStr2.find(" ");
		if(auxStr=="Línea")
		{
			posicion=auxStr2.find(" ");
			auxStr2=auxStr2.substr(0,posicion);
			if(auxStr2=="4a")
			{
				lineaAux=3;
			}
			else
			{
				lineaAux=atoi(auxStr2.c_str());
			}
			aux.id=0;
			aux.linea=0;
			aux.nombre="";
			aux.cod="";
			aux.comb=0;
			metro.push_back(aux);
		}
		else
		{
			aux.id=contador;
			aux.linea=lineaAux;
			aux.nombre=auxStr2;
			aux.cod=auxStr;
			aux.comb=0;
			contador++;
			metro.push_back(aux);
		}
	}
	metro.pop_back();
	Estacion aux2;
	aux2.id=0;
	aux2.linea=0;
	aux2.nombre="";
	aux2.cod="";
	aux2.comb=0;
	metro.push_back(aux2);
}



int BusquedaEstaciones(vector <Estacion> &metro, string Inicio, string Destino, int *inicio, int *final)
{
	int x=0;
	for(int i=0;i<metro.size();i++)
	{
		if(metro[i].id!=0 && metro[i].cod==Inicio)
		{
			*inicio=i;
			x++;
		}
		if(metro[i].id!=0 && metro[i].cod==Destino)
		{
			*final=i;
			x++;
		}
	}
	return x;
}

void SMaster(string &cam,int inic,int contador,int proc)
{
	int l;
	l=cam.length()+1;
	char todo[l];
	cam.copy(todo,l);
	todo[l-1]='\0';
	MPI_Send(&contador, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
	MPI_Send(&inic, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
	MPI_Send(&l, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
	MPI_Send(&todo, l, MPI_CHAR, proc, 0, MPI_COMM_WORLD);
}

void Rmaster(vector <string> &p, vector <int> &v, int proc)
{
	string datos;
	int contador,c_todo;
	MPI_Recv(&contador, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&c_todo, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
	char todo[c_todo];
	MPI_Recv(&todo, c_todo, MPI_CHAR, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
	datos=todo;
	p.push_back(datos);
	v.push_back(contador);
}

void oMaster(vector <Estacion> metro, int *contador, string &cam, int inic, int des, vector <string> &p, vector <int> &v,int tam)
{
	int m_dest,m_inic,c_dest,c_inic,c_dest2=0,c_inic2=0,proc=1,cGlobal=1;
	string camD,camI;

		if(*contador==0 && metro[inic].comb!=0)
		{
			for(int i=0;i<metro.size();i++)
			{
				if(metro[inic].nombre==metro[i].nombre && inic!=i)
				{
					SMaster(cam,i,0,proc);
					proc++;
					cGlobal++;
					i=metro.size();
				}
			}
		}
		c_dest=*contador;
		camD=cam;
		m_dest=inic+1;
		while(metro[m_dest].id!=0 && c_dest2==0)
		{
			c_dest++;
			camD=camD+"-->"+metro[m_dest].nombre;
			if(m_dest==des)
			{
				c_dest2=1;
				p.push_back(camD);
				v.push_back(c_dest);
			}
			else
			{
				if(metro[m_dest].comb!=0)
				{
					for(int i=0;i<metro.size();i++)
					{
						if(metro[m_dest].nombre==metro[i].nombre && m_dest!=i)
						{
							if(metro[i].nombre==metro[des].nombre)
							{
								c_dest2=1;
								p.push_back(camD);
								v.push_back(c_dest);
								i=metro.size();
							}
							else
							{
								if(cGlobal>=tam)
								{
									if(proc==tam)
									proc=1;
									Rmaster(p,v,proc);
								}
								SMaster(camD,i,c_dest,proc);
								proc++;
								cGlobal++;
								i=metro.size();
							}
						}
					}
				}
			}
			
			m_dest++;
		}
		c_inic=*contador;
		camI=cam;
		m_inic=inic-1;
		while(metro[m_inic].id!=0 && c_inic2==0)
		{
			c_inic++;
			camI=camI+"-->"+metro[m_inic].nombre;
			if(m_inic==des)
			{
				p.push_back(camI);
				v.push_back(c_inic);
				c_inic2=1;
			}
			else
			{
				if(metro[m_inic].comb!=0)
				{
					for(int i=0;i<metro.size();i++)
					{
						if(metro[m_inic].nombre==metro[i].nombre && m_inic!=i)
						{
							if(metro[i].nombre==metro[des].nombre)
							{
								p.push_back(camI);
								v.push_back(c_inic);
								c_inic2=1;
								i=metro.size();
							}
							else
							{
								if(cGlobal>=tam)
								{
									if(proc==tam)
									proc=1;
									Rmaster(p,v,proc);
								}
								SMaster(camI,i,c_inic,proc);
								proc++;
								cGlobal++;
								i=metro.size();
							}
						}
					}
				}
			}
			m_inic--;
		}
		if(cGlobal<tam)
		{
			for(int i=1;i<cGlobal;i++)
			{
				Rmaster(p,v,i);
			}
		}
		else
		{
			for(int i=1;i<tam;i++)
			{
				Rmaster(p,v,i);
			}
		}

		for(int i=1;i<tam;i++)
		{
			int nums=-1;
			MPI_Send(&nums, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	
}

void oEsclavo(vector <Estacion> metro, int *contador, string &cam, int inic, int des, vector <string> &p, vector <int> &v)
{

	if(*contador>50)
	{
		*contador=1000;
	}
	else
	{
			int m_dest,m_inic,c_dest,c_inic,c_dest2=0,c_inic2=0;
	string camD,camI;
		c_dest=*contador;
		camD=cam;
		m_dest=inic+1;
		while(metro[m_dest].id!=0 && c_dest2==0)
		{
			c_dest++;
			camD=camD+"->"+metro[m_dest].nombre;
			if(m_dest==des)
			{
				c_dest2=1;
				p.push_back(camD);
				v.push_back(c_dest);
			}
			else
			{
				if(metro[m_dest].comb!=0)
				{
					for(int i=0;i<metro.size();i++)
					{
						if(metro[m_dest].nombre==metro[i].nombre && m_dest!=i)
						{
							if(metro[i].nombre==metro[des].nombre)
							{
								c_dest2=1;
								p.push_back(camD);
								v.push_back(c_dest);
								i=metro.size();
							}
							else
							{
								oEsclavo(metro,&c_dest,camD,i,des,p,v);
							}
						}
					}
				}
			}
			
			m_dest++;
		}
		c_inic=*contador;
		camI=cam;
		m_inic=inic-1;
		while(metro[m_inic].id!=0 && c_inic2==0)
		{
			c_inic++;
			camI=camI+"->"+metro[m_inic].nombre;
			if(m_inic==des)
			{
				p.push_back(camI);
				v.push_back(c_inic);
				c_inic2=1;
			}
			else
			{
				if(metro[m_inic].comb!=0)
				{
					for(int i=0;i<metro.size();i++)
					{
						if(metro[m_inic].nombre==metro[i].nombre && m_inic!=i)
						{
							if(metro[i].nombre==metro[des].nombre)
							{
								p.push_back(camI);
								v.push_back(c_inic);
								c_inic2=1;
								i=metro.size();
							}
							else
							{
								oEsclavo(metro,&c_inic,camI,i,des,p,v);
							}
						}
					}
				}
			}
			m_inic--;
		}
	}
}

void rEsclavo(string camino, int valor, int proc)
{
	int l;
	l=camino.length()+1;
	camino.size();
	char todo[l];
	camino.copy(todo,l);
	todo[l-1]='\0';
	MPI_Send(&valor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	MPI_Send(&l, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	MPI_Send(&todo, l, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
}

void PL_Esclavo(vector <Estacion> metro, int m_destino, int procesador)
{
	vector <string> aux;
	vector <int> auxInt;
	string auxStr;
	int c_todo,c=0,min,ind,m_inicio,op=0;
	while(c!=-1)
	{
		min=1000;
		MPI_Recv(&c, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
		if(c!=-1)
		{
			MPI_Recv(&m_inicio, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
			MPI_Recv(&c_todo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
			char todo[c_todo];
			MPI_Recv(&todo, c_todo, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
			auxStr=todo;
			oEsclavo(metro,&c,auxStr,m_inicio,m_destino,aux,auxInt);
			for(int i=op;i<auxInt.size();i++)
			{
				if(auxInt[i]<min)
				{
					min=auxInt[i];
					ind=i;
				}
			}
			rEsclavo(aux[ind],auxInt[ind],procesador);
			op=auxInt.size();
		}
	}
}

void PL_M(vector <Estacion> metro, int m_inicio, int m_destino, int tam)
{

	int aux=0;
	if(metro[m_inicio].nombre==metro[m_destino].nombre)
	{
		cout<<"La ruta es : "<<endl;
		cout<<metro[m_inicio].nombre<<endl;	
		aux=-1;
		for(int i=1;i<tam;i++)
		{
			MPI_Send(&aux, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	}
	else
	{
		string recorrido=metro[m_inicio].nombre;
		vector <string> auxStr;
		vector <int> auxInt;
		int min=1000;
		oMaster(metro,&aux,recorrido,m_inicio,m_destino,auxStr,auxInt,tam);
		for(int i=0;i<auxInt.size();i++)
		{
			if(auxInt[i]<min)
			{
				min=auxInt[i];
			}
		}
		for(int i=0;i<auxInt.size();i++) //verifica si existen mas de 1 ruta menor y entrega todas las opciones
		{
			if(auxInt[i]==min)
			{
				cout<<"La ruta es :"<<endl;
				cout<<auxStr[i]<<endl;
				cout<<endl;
			}
		}
	}
}


int main(int argc, char* argv[])
{
	t0=clock();
	vector <Estacion> metro;
	string inic,final,argu;
	int in,fin,tam,procesador;
  	MPI_Init(&argc,&argv);
  	MPI_Comm_size(MPI_COMM_WORLD, &tam); 
  	MPI_Comm_rank(MPI_COMM_WORLD, &procesador);
	if(tam>1)
	{
		if(argc==2 && procesador==0) 
		{
			argu=argv[1];
			if(argu=="-v")
			{
				cout<<"Fabian Cancino Riquelme"<<endl;
				cout<<"Victor Gomez Espinosa"<<endl;
				cout<<"Alberto Vasquez Benavente"<<endl;
			}
			else
			{
				cout<<"Entrada Invalida"<<endl;
			}
		}
		else
		{
			if(argc==4)
			{
				argu=argv[1];
				if(argu=="-f")
				{
					inic=argv[2];
					final=argv[3];
					Llenado(metro); //llena el vector de estaciones
					for(int i=0;i<metro.size();i++)
					{
						for(int j=0;j<metro.size();j++)
						{
							if(i!=j && metro[i].id!=0 && metro[j].id!=0 )
							{
								if(metro[i].nombre==metro[j].nombre && metro[i].comb==0)
								{
									metro[i].comb=metro[j].linea;
									metro[j].comb=metro[i].linea;
								}
							}
						}
					}
					
					if(BusquedaEstaciones(metro,inic,final,&in,&fin)==2)
					{	
						if(procesador==0)
						{
							PL_M(metro,in,fin,tam);//planifica 
						}
						else
						{
							PL_Esclavo(metro,fin,procesador);
						}
						
					}
					else
					{
						if(procesador==0)
						cout<<"Verifique codigo estaciones"<<endl;
					}
				}
				else
				{
					if(procesador==0)
					cout<<"Entrada Invalida"<<endl;
				}
			}
			else
			{
				if(procesador==0)
				cout<<"Error en datos ingresados"<<endl;
			}
		}
	}
	else
	{
		cout<<"Se debe usar mas de 1 procesador"<<endl;
	}
	
	MPI_Finalize();
	t1=clock();
    double time = (double(t1-t0)/CLOCKS_PER_SEC);
    cout << "Execution Time: " << time << endl;
}