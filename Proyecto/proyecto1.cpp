#include<iostream>
using namespace std;
const int MAX=100;
void correspondenciaDirecta(int numBloque[MAX], int n);
void asociativaDeDosVias(int numBloque[MAX], int n);
void completamenteAsociativa(int numBloque[MAX], int n);
int esta(int conjuntoARev[4], int valor);
int main(){
	int dirBloque[MAX];
	int num,j,i,casosPrueba;
	
	cin>>casosPrueba;
	for (i=0;i<casosPrueba;i++){
		cin>>num;
		for(j=0;j<num;j++){
			cin>>dirBloque[j];
			}
		cout<<"\nEl Resultado de la simulacion usando Cache de Correspondencia Directa es:\n\n";
		correspondenciaDirecta(dirBloque,num);
		
		cout<<"\nEl Resultado de la simulacion usando Cache Asociativa de dos Vias es:\n\n";
		asociativaDeDosVias(dirBloque,num);
		
		cout<<"\nEl Resultado de la simulacion usando Cache Completamente Asociativa es:\n\n";
		completamenteAsociativa(dirBloque,num);
	}
		
	
	return 0;
	}

void correspondenciaDirecta(int numBloque[MAX], int n){
	int bloqueCacheVal[MAX];
	int cacheCorrDir[MAX];
	int i;
	
	//calculando en que bloque se puede guardar cada direccion e inicializando cache de correspondencia directa
	for (i=0;i<n;i++){
		bloqueCacheVal[i]=numBloque[i]%4;
		cacheCorrDir[i]=-1;
		};
	for (i=0;i<n;i++){
		if(numBloque[i]==cacheCorrDir[bloqueCacheVal[i]]){
			cout<<"	ACIERTO\n";
		}else{
			cout<<" FALLO\n";
			cacheCorrDir[bloqueCacheVal[i]]=numBloque[i];
			}
		}
	}

void asociativaDeDosVias(int numBloque[MAX], int n){
	int conjunto0[2];
	int conjunto1[2];
	int conjVal[MAX];
	int i, mrcj1,mrcj0;//mrcj= mas reciente del conjunto
	
	
	for(i=0;i<n;i++){
		conjVal[i]=numBloque[i]%2;
		}
	for(i=0;i<2;i++){
		conjunto0[i]=-1;
		conjunto1[i]=-1;
		}
	mrcj0=-1;
	mrcj1=-1;
	for(i=0;i<n;i++){
		if(conjVal[i]==0){
			if(numBloque[i]==conjunto0[0]||numBloque[i]==conjunto0[1]){
				cout<<"ACIERTO\n";
			}else{
				cout<<"FALLO\n";
				if(conjunto0[0]==mrcj0){
					conjunto0[1]=numBloque[i];
				}else{
					conjunto0[0]=numBloque[i];
					}
				}
			mrcj0=numBloque[i];
		}else{
			if(numBloque[i]==conjunto1[0]||numBloque[i]==conjunto1[1]){
				cout<<"ACIERTO\n";
			}else{
				cout<<"FALLO\n";
				if(conjunto1[0]==mrcj1){
					conjunto1[1]=numBloque[i];
				}else{
					conjunto1[0]=numBloque[i];
					}
				}
			mrcj1=numBloque[i];
			}
		}
}

void completamenteAsociativa(int numBloque[MAX], int n){
	int conjunto[4];
	int i,bandAux,j, ultLlamado;
	
	for(i=0;i<5;i++){
		conjunto[i]=-1;
		}
	ultLlamado=-1;
	j=0;
	for (i=0;i<n;i++){
		if(esta(conjunto,numBloque[i])==1){
			cout<<"ACIERTO\n";
		}else{
			cout<<"FALLO\n";
			if(j==n-1){
				j=0;
				bandAux=0;
				while(bandAux==0){
					if(conjunto[j]!=ultLlamado){
						conjunto[j]=numBloque[i];
						bandAux=1;
						}
					j=j+1;}
			}else{
				j=j+1;
				conjunto[j]=numBloque[i];
				}
			ultLlamado=numBloque[i];
		}
		}
	}

int esta(int conjuntoARev[4], int valor){
	int j,bandera;
	bandera=0;
	j=0;
	while(j<4&&bandera==0){
		if(conjuntoARev[j]==valor){
			bandera=1;
			}
		j++;
		}
	return bandera;
	}
