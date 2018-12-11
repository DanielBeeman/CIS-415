#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#define NUMTOPICS 1
#define MAXENTRIES 100
#define QUACKSIZE 140
#define DELTA 15

int circular_buffer;

pthread_t publisher;
pthread_t subscriber;
pthread_t delete;

struct topicentry{
    int entrynum;
    struct timeval timestamp;
    int pubID;
    char *message;					//will want to change this for char *
};


struct queue{
    pthread_mutex_t topic_lock;		 //the lock on the topic, so we don't en/dequeue akwardly 
    struct topicentry circular_buffer[MAXENTRIES]; 	//maximum number of entries for this queue
    int in;  						//the tail, where we will add to the queue
    int out; 						//front of the queue that we will dequeue
    int topic_counter; 					//to count num. entries in a subject
};

struct queue Topic_Q_ptr;				

int enqueue(struct queue *Q_ptr, struct topicentry *new_post)
{
    pthread_mutex_lock(&Q_ptr -> topic_lock);
    if( (Q_ptr -> in + 1) % MAXENTRIES == Q_ptr -> out )	//checks if buffer is full. 
    {
        /*
	printf("\nQ_ptr -> in                      = %d", Q_ptr -> in);
        printf("\nMAXENTRIES                       = %d", MAXENTRIES);
        printf("\n(Q_ptr -> in + 1) mod MAXENTRIES = %d\n", ((Q_ptr -> in + 1) % MAXENTRIES));
        printf("\nQ_ptr -> out                     = %d\n", Q_ptr -> out);
        Buffer is full. 
        Return -1 and allow caller to try again. */
        pthread_mutex_unlock(&Q_ptr -> topic_lock);		//if the buffer is full, unlock.
        return -1;						//returns -1 if failed to queue.
    }

    Q_ptr -> topic_counter += 1;			//increment # of entries in this topic
    new_post -> entrynum = Q_ptr -> topic_counter;      //associates num of topic entry w/ queue num

    struct timeval time;
    gettimeofday(&time, NULL);	
    new_post -> timestamp = time;			//gets time of day, assigns to topic entry

    new_post -> pubID = 0;
    new_post -> message = "Season's Greetings!";
    Q_ptr -> circular_buffer[Q_ptr -> in] = *new_post;  //sets tail of queue to the new post
    Q_ptr -> in = (Q_ptr -> in + 1) % MAXENTRIES;
    pthread_mutex_unlock(&Q_ptr -> topic_lock);		//after adding to the queue unlock the queue
    printf("\n");
    printf("----------\n");
    printf("Publisher thread 1 posted new entry in topic 1\n");
    printf("Next entry added will be placed at index %d\n", Q_ptr -> in);
    printf("Next entry removed will be come from index %d\n", Q_ptr -> out);
    printf("Highest topic number is %d\n", Q_ptr -> topic_counter);
    printf("Message said %s\n", new_post -> message);
    //printf("Posted %s\n", new_post -> timestamp);
    printf("Entry number %d\n", new_post -> entrynum);
    printf("----------\n");
    printf("\n");
    return 0;						//returns 0 if successfully queued, -1 else
}

int dequeue(struct queue * Q_ptr)
{
    pthread_mutex_lock(&Q_ptr -> topic_lock);		//lock the queue
    int o = Q_ptr -> out;
    int i = Q_ptr -> in;
    if (i == o)						//If the queue is already empty
    {
        //printf("empty queue\n"); 
        pthread_mutex_unlock(&Q_ptr -> topic_lock);
       	return -1;					//unlock, return -1
    }
    /*
    Move out to the next spot in the queue.		//missing code! need to dequeue and move
    */							//to next entry in queue

    //now we need to handle when an entry has been in the queue for a while
    struct timeval diff;
    struct timeval time;
    gettimeofday(&time, NULL);

    timersub(&time, &(Q_ptr -> circular_buffer[Q_ptr -> out].timestamp), &diff);
    //printf("printing difference: %d\n", diff);
    if (diff.tv_sec > DELTA) {
	printf("\n");
	printf("----------\n");
	printf("Clean-up thread 1 deleted topic entry %d\n", Q_ptr-> circular_buffer[Q_ptr -> out].entrynum);
	printf("message was %d seconds old.\n", diff);
	Q_ptr -> out = (Q_ptr -> out + 1) % MAXENTRIES;
	printf("next entry will be placed at index %d\n", Q_ptr -> out);
	printf("next entry will be removed from index %d\n", Q_ptr -> out);
	printf("----------\n");
	printf("\n");
        pthread_mutex_unlock(&Q_ptr -> topic_lock);
	return 1;
    }

    else {
	//printf("nothing deleted!\n");
	pthread_mutex_unlock(&Q_ptr -> topic_lock);
	return -1;
    }
    /*
    int data = Q_ptr -> circular_buffer[Q_ptr -> out].message;
    Q_ptr -> out = (Q_ptr -> out + 1) % MAXENTRIES;
    pthread_mutex_unlock(&Q_ptr -> topic_lock);
    printf("out entrynum: %d\n", Q_ptr -> circular_buffer[o].entrynum);
    return data;
    */
}

int read_post(struct queue * Q_ptr, int lastentry, struct topicentry *t) 
{							//want to read, then move to next entry.
    							//if entry has been deleted, need to try
	
	int o = Q_ptr -> out;
	int i = Q_ptr -> in;
	pthread_mutex_lock(&Q_ptr -> topic_lock);
	if (i == o) {		//handles when queue emptied
		int val;
		val = lastentry * (-1);
		//printf("tried to read, Empty queue\n");
		pthread_mutex_unlock(&Q_ptr -> topic_lock);
		return val;
	}
	int curr_ent = lastentry + 1;			//when entry we want is in queue
	//NEED TO ACCOUNT FOR IF READING FIRST POST	
	if ((curr_ent) >= (Q_ptr -> circular_buffer[o].entrynum) && ((curr_ent) <= (Q_ptr -> circular_buffer[i-1].entrynum))) {
		int offset = Q_ptr -> topic_counter - curr_ent; //grab offset of entry we want
		int want_ind = i - offset;		//grabs index in buff we want

		if (want_ind < 0) {				//if we have wrapped around buff
			want_ind = MAXENTRIES + want_ind;		//gets new index we need
		}
		t-> message = Q_ptr -> circular_buffer[want_ind].message; //grabs new message, sets
		t -> entrynum = Q_ptr -> circular_buffer[want_ind-1].entrynum;
		pthread_mutex_unlock(&Q_ptr -> topic_lock);
		//printf("When entry we want is in the queue\n");
		return 1;				
	}
	else if ((curr_ent - 1) == Q_ptr -> circular_buffer[i-1].entrynum){
		//printf("already read message at the end of the queue\n");
		t-> message = Q_ptr -> circular_buffer[i-1].message;
 		t-> entrynum = Q_ptr -> circular_buffer[i-1].entrynum;
		pthread_mutex_unlock(&Q_ptr -> topic_lock);
		int val;
		val = lastentry * (-1);
		return val;

	}

	
	else {
		/*	
		printf("curr_ent val: %d\n", curr_ent);
		printf("q_ptr o: %d\n", o);
		printf("q_ptr i: %d\n", i);
		printf("out entrynum: %d\n", Q_ptr -> circular_buffer[o].entrynum);
		printf("in entrynum: %d\n", Q_ptr -> circular_buffer[i -1].entrynum);
		
		printf("Entry behind out pointer\n");
		*/
		t -> message = Q_ptr -> circular_buffer[Q_ptr -> out].message;
		t -> entrynum = Q_ptr -> circular_buffer[Q_ptr -> out].entrynum;
		pthread_mutex_unlock(&Q_ptr -> topic_lock);
		return Q_ptr -> circular_buffer[Q_ptr -> out].entrynum;
	}
	
	
	//need too check if entry we are looking for is already dequeued
	//if it is, we need to keep going until we get to next avail. entry
	//can compare topic counter with lastentry

}							//next entry. If no new entry, need to wait

void printtopicQ(struct queue *Q_ptr)
{							//helper function to print out a topic
    printf("in  = %d\n", Q_ptr->in);
    printf("out = %d\n", Q_ptr->out);
}

void* pub()						//publisher creating a message
{
    printf("\n Pub thread running.\n");
    
    pthread_t tid = pthread_self();			//not sure what this line does
    printf("\n thread id = %d.\n", (int)tid);		//thread id =s itself??
    
    
    
    struct topicentry this_topic;
    printf("\n Created topic entry.\n");
    
    Topic_Q_ptr.in =0;
    Topic_Q_ptr.out = 0;

    //printtopicQ(&Topic_Q_ptr);

    printf("\n Creating the topic lock.\n");

    if(pthread_mutex_init(&(Topic_Q_ptr.topic_lock), NULL) != 0) //tries to lock the queue
    {
        printf("\n mutex init has failed\n");
    }
    int i;
    for(i = 0; i < 99; i++)					
    {
        printf(" Creating a topic: %d\n", i);
        this_topic.message = "Season's Greetings!";
        int indicator = enqueue(&Topic_Q_ptr, &this_topic);   //indicates if proc. succ. added to Q
        //printtopicQ(&Topic_Q_ptr);
        while (indicator == -1)				      //if process fails to enqueue
        {
            pthread_yield();
            indicator = enqueue(&Topic_Q_ptr, &this_topic);   //try again to enqueue
	    //printtopicQ(&Topic_Q_ptr);
        }
        //printf("successfully added to the queue: %d\n", indicator);
    }
    sleep(10);
    
    for(i = 0; i < 5; i++)					
    {
        printf(" Creating a topic: %d\n", i);
        this_topic.message = "Season's Greetings!";
        int indicator = enqueue(&Topic_Q_ptr, &this_topic);   //indicates if proc. succ. added to Q
        //printtopicQ(&Topic_Q_ptr);
        while (indicator == -1)				      //if process fails to enqueue
        {
            pthread_yield();
            indicator = enqueue(&Topic_Q_ptr, &this_topic);   //try again to enqueue
	    //printtopicQ(&Topic_Q_ptr);
        }
        //printf("successfully added to the queue: %d\n", indicator);
    }
    for(i = 0; i < 20; i++)					
    {
	sleep(1);
        printf(" Creating a topic: %d\n", i);
        this_topic.message = "Season's Greetings!";
        int indicator = enqueue(&Topic_Q_ptr, &this_topic);   //indicates if proc. succ. added to Q
        //printtopicQ(&Topic_Q_ptr);
        while (indicator == -1)				      //if process fails to enqueue
        {
            pthread_yield();
            indicator = enqueue(&Topic_Q_ptr, &this_topic);   //try again to enqueue
	    //printtopicQ(&Topic_Q_ptr);
        }
        //printf("successfully added to the queue: %d\n", indicator);
    }
    pthread_mutex_destroy(&(Topic_Q_ptr.topic_lock));	      //remove and delete lock
    return NULL;

}

void* sub()
{
    printf("\n Sub thread running.\n");
    
    pthread_t tid = pthread_self();			//not sure what this line does
    printf("\n thread id = %d.\n", (int)tid);		//thread id =s itself??
    
    struct topicentry this_topic;
    printf("\n Created topic entry.\n");
    
    Topic_Q_ptr.in =0;
    Topic_Q_ptr.out = 0;

    //printtopicQ(&Topic_Q_ptr);

    int lastent;
    lastent = 0;		//lastent initialized to 0

    int i;
    while (1){
	sleep(1);
        for(i = 0; i < 6; i++)					
        {
            //printf("\n Reading a topic: %d\n", i);
            //this_topic.message = "Season's Greetings!";
            int indicator = read_post(&Topic_Q_ptr, lastent, &this_topic);   //Attempt to read post
	    struct timeval first;
    	    gettimeofday(&first, NULL);
            //printtopicQ(&Topic_Q_ptr);
            while (indicator < 1)				      //empty queue
            {						//now we check if we have waited 10 secs
		struct timeval diff;
    		struct timeval time;
    		gettimeofday(&time, NULL);

    		timersub(&time, &first, &diff);
		if (diff.tv_sec > 10) {
			printf("Sub has not read in %d seconds\n", diff.tv_sec);
			printf("Sub has been terminated.\n");
			return 0;	
		}
                lastent = indicator * (-1);
	        pthread_yield();
                indicator = read_post(&Topic_Q_ptr, lastent, &this_topic);   //try to read again
                //printf("stuck in sub\n");
	        //printtopicQ(&Topic_Q_ptr);
            }
	    if (indicator == 1) {					// we found our entry in Q
	        lastent += 1;
	    }
	    if (indicator > 1) {
	        lastent = indicator;
	    }
	    printf("\n");
 	    printf("----------\n");
            printf("Subscriber Thread 1 read an entry from topic 1\n");
	    printf("Next entry added will be placed at index %d\n", Topic_Q_ptr.in);
	    printf("Next entry removed will come from index %d\n", Topic_Q_ptr.out);
	    printf("Highest topic number is %d\n", Topic_Q_ptr.topic_counter);
	    printf("Message said %s\n", this_topic.message);
	    //printf("Posted %s\n", new_post -> timestamp);
            printf("Entry number %d\n", this_topic.entrynum);
	    //sleep(1);
	    printf("----------\n");
	    printf("\n"); 
        }
    }
    return NULL;
}

void* del()
{

//need to go over timing stuff, especially when indicator < 1.  
    int i;
    while(1) {
        int indicator = dequeue(&Topic_Q_ptr);
	struct timeval first;
    	gettimeofday(&first, NULL);
	while (indicator < 1) 
        {
	    struct timeval diff;
    	    struct timeval time;
    	    gettimeofday(&time, NULL);

    	    timersub(&time, &first, &diff);
	    if (diff.tv_sec > 30) {
		printf("delete has not deleted in %d seconds. terminated.\n", diff.tv_sec);
		return 0;	
	    }
	    pthread_yield();
	    //printf("unable to dequeue\n");
	    //printtopicQ(&Topic_Q_ptr);
	    indicator = dequeue(&Topic_Q_ptr);
	}
    }
}

int main(int argc, char *argsv[])
{
    int num_pubs = 1;
    int num_subs = 1;
    int error;

    error = pthread_create(&publisher, NULL, &pub, NULL);
    if (error != 0)
        printf("\n Thread can't be created : [%s]\n", strerror(error));
    
    error = pthread_create(&subscriber, NULL, sub, NULL);
    if (error != 0)
    	printf("\n Thread can't be created : [%s]", strerror(error));
   
    error = pthread_create(&delete, NULL, del, NULL);
    if (error != 0)
    	printf("\n Thread can't be created : [%s]", strerror(error));
    
    pthread_join(publisher, NULL);
    pthread_join(subscriber, NULL);
    pthread_join(delete, NULL);
    return 0;
}
