// MIT License
// 
// Copyright (c) 2023 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include "utility.h"
#include "star.h"
#include "float.h"
#include <pthread.h>

#define NUM_STARS 30000 
#define MAX_LINE 1024
#define DELIMITER " \t\n"

struct Star star_array[ NUM_STARS ];
uint8_t   (*distance_calculated)[NUM_STARS];

pthread_mutex_t mutex;

double  min  = FLT_MAX;
double  max  = FLT_MIN;

typedef struct ThreadStruct
{
  int id;
  uint32_t thread_start;
  uint32_t thread_end;
  double mean;
  uint64_t count;

} ThreadStruct;


void * threadFunc( void * arg )
{
  ThreadStruct * var = (ThreadStruct*)arg;

  double mean = 0;
  double local_min = FLT_MAX;
  double local_max = FLT_MIN;

  uint32_t i, j;
  uint64_t count = 0;

  for (i = var->thread_start; i < var->thread_end; i++)
  {
    for (j = i+1; j < NUM_STARS; j++)
    {
      if( i!=j && distance_calculated[i][j] == 0 )
      {
        
        double distance = calculateAngularDistance( star_array[i].RightAscension, star_array[i].Declination,
                                                    star_array[j].RightAscension, star_array[j].Declination ) ;
        
        
        // pthread_mutex_lock(&mutex);
        distance_calculated[i][j] = 1;
        distance_calculated[j][i] = 1;
        // pthread_mutex_unlock(&mutex);
        count++;

        if( local_min > distance )
        {
          local_min = distance;
        }
        if( local_max < distance )
        {
          local_max = distance;
        }
        mean = mean + (distance-mean)/count;
        
      }
    }
  }
  
  if( min > local_min )
  {
    pthread_mutex_lock(&mutex);
    min = local_min;
    pthread_mutex_unlock(&mutex);

  }
  if( max < local_max )
  {
    pthread_mutex_lock(&mutex);
    max = local_max;
    pthread_mutex_unlock(&mutex);

  }
  var->mean = mean; // return mean on per thread basis
  var->count = count; // return count
  return NULL;  
  
}

void showHelp()
{
  printf("Use: findAngular [options]\n");
  printf("Where options are:\n");
  printf("-t          Number of threads to use\n");
  printf("-h          Show this help\n");
}

float wrapperControlFunc( struct Star arr[],pthread_t * thread_id, ThreadStruct * var, int num_threads )
{
  uint32_t workload = NUM_STARS / num_threads;

  double mean = 0, total_sum=0;
  uint64_t total_count = 0;

  for (int i = 0; i < num_threads; i++)
  {
    var[i].id = i;
    var[i].thread_start = i * workload;
    var[i].thread_end = (i+1) * workload;

    //inacase the number of threads is not a factor of total number of stars
    if( i == num_threads - 1 )
    {
      var[i].thread_end = NUM_STARS;
    }
  }

  for (int i = 0; i < num_threads; i++)
  {
    pthread_create( &thread_id[i], NULL, threadFunc, (void*)&var[i] );
  }

  for (int i = 0; i < num_threads; i++)
  {
    pthread_join( thread_id[i], NULL );
  }

  // merging the means of all threads to get the final mean
  for (int i = 0; i < num_threads; i++)
  {
    total_count += var[i].count;
    total_sum += var[i].mean * var[i].count;
  }
  mean = total_sum / total_count; 

  return mean;
}


int main( int argc, char * argv[] )
{
  FILE *fp;
  uint32_t star_count = 0,n;

  int num_threads = 1; // default to 1 thread

  distance_calculated = malloc(sizeof(uint8_t[NUM_STARS][NUM_STARS]));

  if( distance_calculated == NULL )
  {
    uint64_t num_stars = NUM_STARS;
    uint64_t size = num_stars * num_stars * sizeof(uint8_t);
    printf("Could not allocate %lu bytes\n", size);
    exit( EXIT_FAILURE );
  }

  int i, j, flag = 0;
  
  //changed the for loop to memset to initialize the array to 0
  memset( (void*)distance_calculated, 0, sizeof(uint8_t[NUM_STARS][NUM_STARS]) );

  for( n = 1; n < argc; n++ )          
  {
    if( strcmp(argv[n], "-help" ) == 0 )
    {
      showHelp();
      exit(0);
    }
  }

  for (n=1; n < argc; n++)
  {
    if(( strcmp(argv[n], "-t" ) == 0 ) && flag == 0)
    {
      num_threads = atoi(argv[n+1]);
      flag = 1;
      printf("Using %d threads\n", num_threads);
    }
  }


  fp = fopen( "data/tycho-trimmed.csv", "r" );

  if( fp == NULL )
  {
    printf("ERROR: Unable to open the file data/tycho-trimmed.csv\n");
    exit(1);
  }

  char line[MAX_LINE];
  while (fgets(line, 1024, fp))
  {
    uint32_t column = 0;

    char* tok;
    for (tok = strtok(line, " ");
            tok && *tok;
            tok = strtok(NULL, " "))
    {
       switch( column )
       {
          case 0:
              star_array[star_count].ID = atoi(tok);
              break;
       
          case 1:
              star_array[star_count].RightAscension = atof(tok);
              break;
       
          case 2:
              star_array[star_count].Declination = atof(tok);
              break;

          default: 
             printf("ERROR: line %d had more than 3 columns\n", star_count );
             exit(1);
             break;
       }
       column++;
    }
    star_count++;
  }
  printf("%d records read\n", star_count );
  pthread_t tid [num_threads];
  ThreadStruct var[num_threads];

  double distance =  wrapperControlFunc( star_array,tid, var, num_threads );

  printf("Average distance found is %lf\n", distance );
  printf("Minimum distance found is %lf\n", min );
  printf("Maximum distance found is %lf\n", max );


  free(distance_calculated);
  fclose(fp);


  return 0;
}

