#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include<mpi.h>
using namespace std;

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
	int posicion,cont=1,lineaAux;
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
			aux.Abrev="";
			aux.combinacion=0;
			metro.push_back(aux);
		}
		else
		{
			aux.id=cont;
			aux.linea=lineaAux;
			aux.nombre=auxStr2;
			aux.Abrev=auxStr;
			aux.combinacion=0;
			cont++;
			metro.push_back(aux);
		}
	}
	metro.pop_back();
	Estacion aux2;
	aux2.id=0;
	aux2.linea=0;
	aux2.nombre="";
	aux2.Abrev="";
	aux2.combinacion=0;
	metro.push_back(aux2);
}



int BusquedaEstaciones(vector <Estacion> &metro, string Inicio, string Destino, int *inicio, int *final)
{
	int x=0;
	for(int i=0;i<metro.size();i++)
	{
		if(metro[i].id!=0 && metro[i].Abrev==Inicio)
		{
			*inicio=i;
			x++;
		}
		if(metro[i].id!=0 && metro[i].Abrev==Destino)
		{
			*final=i;
			x++;
		}
	}
	return x;
}

void envia_Maestro(string &cam,int inic,int cont,int proc)
{
	int largo;
	largo=cam.length()+1;
	char todo[largo];
	cam.copy(todo,largo);
	todo[largo-1]='\0';
	MPI_Send(&cont, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
	MPI_Send(&inic, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
	MPI_Send(&largo, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
	MPI_Send(&todo, largo, MPI_CHAR, proc, 0, MPI_COMM_WORLD);
}

void recibe_Maestro(vector <string> &posibles, vector <int> &valores, int proc)
{
	string datos;
	int cont,cantodo;
	MPI_Recv(&cont, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&cantodo, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
	char todo[cantodo];
	MPI_Recv(&todo, cantodo, MPI_CHAR, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
	datos=todo;
	posibles.push_back(datos);
	valores.push_back(cont);
	//cout<<"PROCESADOR "<<proc<<endl;
	//cout<<endl;
	//cout<<datos<<endl;

}

void opciones_Maestro(vector <Estacion> metro, int *cont, string &cam, int inic, int des, vector <string> &posibles, vector <int> &valores,int tamano)
{
	int movd,movi,contd,conti,checkd=0,checki=0,proc=1,contglob=1;
	string caminod,caminoi;

		if(*cont==0 && metro[inic].comb!=0)
		{
			for(int i=0;i<metro.size();i++)
			{
				if(metro[inic].nombre==metro[i].nombre && inic!=i)
				{
					envia_Maestro(cam,i,0,proc);
					proc++;
					contglob++;
					i=metro.size();
				}
			}
		}
		contd=*cont;
		caminod=cam;
		movd=inic+1;
		while(metro[movd].id!=0 && checkd==0)
		{
			contd++;
			caminod=caminod+"->"+metro[movd].nombre;
			if(movd==des)
			{
				checkd=1;
				posibles.push_back(caminod);
				valores.push_back(contd);
			}
			else
			{
				if(metro[movd].comb!=0)
				{
					for(int i=0;i<metro.size();i++)
					{
						if(metro[movd].nombre==metro[i].nombre && movd!=i)
						{
							if(metro[i].nombre==metro[des].nombre)
							{
								checkd=1;
								posibles.push_back(caminod);
								valores.push_back(contd);
								i=metro.size();
							}
							else
							{
								if(contglob>=tamano)
								{
									if(proc==tamano)
									proc=1;
									recibe_Maestro(posibles,valores,proc);
								}
								envia_Maestro(caminod,i,contd,proc);
								proc++;
								contglob++;
								i=metro.size();
							}
						}
					}
				}
			}
			
			movd++;
		}
		conti=*cont;
		caminoi=cam;
		movi=inic-1;
		while(metro[movi].id!=0 && checki==0)
		{
			conti++;
			caminoi=caminoi+"->"+metro[movi].nombre;
			if(movi==des)
			{
				posibles.push_back(caminoi);
				valores.push_back(conti);
				checki=1;
			}
			else
			{
				if(metro[movi].comb!=0)
				{
					for(int i=0;i<metro.size();i++)
					{
						if(metro[movi].nombre==metro[i].nombre && movi!=i)
						{
							if(metro[i].nombre==metro[des].nombre)
							{
								posibles.push_back(caminoi);
								valores.push_back(conti);
								checki=1;
								i=metro.size();
							}
							else
							{
								if(contglob>=tamano)
								{
									if(proc==tamano)
									proc=1;
									recibe_Maestro(posibles,valores,proc);
								}
								envia_Maestro(caminoi,i,conti,proc);
								proc++;
								contglob++;
								i=metro.size();
							}
						}
					}
				}
			}
			movi--;
		}
		if(contglob<tamano)
		{
			for(int i=1;i<contglob;i++)
			{
				recibe_Maestro(posibles,valores,i);
			}
		}
		else
		{
			for(int i=1;i<tamano;i++)
			{
				recibe_Maestro(posibles,valores,i);
			}
		}

		for(int i=1;i<tamano;i++)
		{
			int nums=-1;
			MPI_Send(&nums, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	
}

void opciones_Esclavo(vector <Estacion> metro, int *cont, string &cam, int inic, int des, vector <string> &posibles, vector <int> &valores)
{

	if(*cont>50)
	{
		*cont=1000;
	}
	else
	{
			int movd,movi,contd,conti,checkd=0,checki=0;
	string caminod,caminoi;
		contd=*cont;
		caminod=cam;
		movd=inic+1;
		while(metro[movd].id!=0 && checkd==0)
		{
			contd++;
			caminod=caminod+"->"+metro[movd].nombre;
			if(movd==des)
			{
				checkd=1;
				posibles.push_back(caminod);
				valores.push_back(contd);
			}
			else
			{
				if(metro[movd].comb!=0)
				{
					for(int i=0;i<metro.size();i++)
					{
						if(metro[movd].nombre==metro[i].nombre && movd!=i)
						{
							if(metro[i].nombre==metro[des].nombre)
							{
								checkd=1;
								posibles.push_back(caminod);
								valores.push_back(contd);
								i=metro.size();
							}
							else
							{
								opciones_Esclavo(metro,&contd,caminod,i,des,posibles,valores);
							}
						}
					}
				}
			}
			
			movd++;
		}
		conti=*cont;
		caminoi=cam;
		movi=inic-1;
		while(metro[movi].id!=0 && checki==0)
		{
			conti++;
			caminoi=caminoi+"->"+metro[movi].nombre;
			if(movi==des)
			{
				posibles.push_back(caminoi);
				valores.push_back(conti);
				checki=1;
			}
			else
			{
				if(metro[movi].comb!=0)
				{
					for(int i=0;i<metro.size();i++)
					{
						if(metro[movi].nombre==metro[i].nombre && movi!=i)
						{
							if(metro[i].nombre==metro[des].nombre)
							{
								posibles.push_back(caminoi);
								valores.push_back(conti);
								checki=1;
								i=metro.size();
							}
							else
							{
								opciones_Esclavo(metro,&conti,caminoi,i,des,posibles,valores);
							}
						}
					}
				}
			}
			movi--;
		}
	}
}

void resultado_Esclavo(string camino, int valor, int proc)
{
	int largo;
	largo=camino.length()+1;
	camino.size();
	char todo[largo];
	camino.copy(todo,largo);
	todo[largo-1]='\0';
	MPI_Send(&valor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	MPI_Send(&largo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	MPI_Send(&todo, largo, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
}

void planviaje_Esclavo(vector <Estacion> metro, int des, int proc)
{
	vector <string> posibles;
	vector <int> valores;
	string datos;
	int cantodo,cont=0,menor,indice,ini,op=0;
	while(cont!=-1)
	{
		menor=999;
		MPI_Recv(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
		if(cont!=-1)
		{
			MPI_Recv(&ini, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
			MPI_Recv(&cantodo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
			char todo[cantodo];
			MPI_Recv(&todo, cantodo, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
			datos=todo;
			opciones_Esclavo(metro,&cont,datos,ini,des,posibles,valores);
			for(int i=op;i<valores.size();i++)
			{
				if(valores[i]<menor)
				{
					menor=valores[i];
					indice=i;
				}
			}
			resultado_Esclavo(posibles[indice],valores[indice],proc);
			op=valores.size();
		}
	}
}

void planviaje_Maestro(vector <Estacion> metro, int ini, int des, int tamano)
{

	int cont=0;
	if(metro[ini].nombre==metro[des].nombre)
	{
		cout<<"_________________RUTA PLANIFICADA___________________"<<endl;
		cout<<metro[ini].nombre<<endl;	
		cont=-1;
		for(int i=1;i<tamano;i++)
		{
			MPI_Send(&cont, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	}
	else
	{
		string recorrido=metro[ini].nombre;
		vector <string> posibles;
		vector <int> valores;
		int menor=999;
		opciones_Maestro(metro,&cont,recorrido,ini,des,posibles,valores,tamano);
		for(int i=0;i<valores.size();i++)
		{
			if(valores[i]<menor)
			{
				menor=valores[i];
			}
		}
		for(int i=0;i<valores.size();i++) //verifica si existen mas de 1 ruta menor y entrega todas las opciones
		{
			if(valores[i]==menor)
			{
				cout<<"_________________RUTA PLANIFICADA___________________"<<endl;
				cout<<posibles[i]<<endl;
				cout<<endl;
			}
		}
	}
}


int main(int argc, char* argv[])
{
	vector <Estacion> metro;
	string inic,final,argu;
	int in,fin,tamano,procesador;
  	MPI_Init(&argc,&argv);
  	MPI_Comm_size(MPI_COMM_WORLD, &tamano); 
  	MPI_Comm_rank(MPI_COMM_WORLD, &procesador);
	if(tamano>1)
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
							planviaje_Maestro(metro,in,fin,tamano);//planifica 
						}
						else
						{
							planviaje_Esclavo(metro,fin,procesador);
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
		cout<<"Utilizar mas de 1 procesador"<<endl;
	}
	
	MPI_Finalize();
}