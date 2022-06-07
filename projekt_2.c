#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define constTime 10000

pthread_mutex_t bridge;
pthread_cond_t gate;
int bridgeDirection = 1;            //direction for thr bridge movement
int limit = 3;	                    //number of cars that can cross the bridge form same side at one change
int carsDrived = 0;	                // counter of cars that cross the bridge

int cityStay[2];                    //number of cars staying in cityX
int cityWait[2];                    //number of cars waiting to leave cityX

void display(int carNumber, int from){
	printf("A-%d %d>>>", cityStay[0], cityWait[0]);
	switch(from)
	{
		case 0:
		printf("[>> %d >>]", carNumber);
		break;
		case 1:
		printf("[<< %d <<]", carNumber);
		break;
	}
	printf("<<<<%d %d-B\n", cityWait[1], cityStay[1] );
}

int opposite(int city){
	return (city+1)%2;
}

void *carFunction(void *arg){
	int nr = (int)arg;
    int city = 0;                                   // 0 - cityA, 1 - cityB
	cityStay[city]++;

	for(;;){
		usleep(constTime+rand()%constTime*1000);	//car is staying in the city for some time
			cityStay[city]--;		                //car signals need for leave and wait in queue
			cityWait[city]++;		                // *
			printf("do kolejki %d", nr);
		pthread_mutex_lock(&bridge);
		while(bridgeDirection==city){	            //if direction of movement is facing towards the car
			pthread_cond_wait(&gate, &bridge);	    //wait for direction change
		}
		cityWait[city]--;		                    //car leave city and enter the bridge
		display(nr,city);		                    //car passes the bridge
		usleep(constTime*100);	                    //*
		city=opposite(city);	                    //car appers on the other side
		cityStay[city]++;		                    //*
		carsDrived++;
		if(carsDrived>limit||cityWait[opposite(bridgeDirection)]==0){	//if no one is left waiting or if limit of cars have been reached
			carsDrived = 0;	                                            //reset counter
			bridgeDirection = opposite(bridgeDirection);	            //change direction of the movement
			pthread_cond_broadcast(&gate);					            //inform waiting cars about change
		}

		pthread_mutex_unlock(&bridge);	                                //unlock bridge
	}
}


int main(int argc, char* argv[]){

	int N = atoi(argv[1]);
	if(N<=0)exit(0);
	
    pthread_t cars[N];

	srand(time(NULL));
	pthread_mutex_init(&bridge,NULL);
	pthread_cond_init(&gate, NULL);

	pthread_mutex_lock(&bridge);
    for(int i =0; i<N; i++){
        pthread_create(cars+i, NULL, carFunction, (void*)i);
    }
	pthread_mutex_unlock(&bridge);


	for(int i =0; i<N; i++){
        pthread_join(cars[i], NULL);
    }    
}
