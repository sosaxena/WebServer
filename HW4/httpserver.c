/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include<pthread.h>

//Global Variables
char*path;

char entire_path[1024]; //Holds the entire path

char content[1024]; //Holds the file content

char user_input[100]; //Input from the user to stop the program
 
char*header; //Header string

char*error_message; //Error message following the header for 404 and 500 errors

int success_count=0; //Counts successful requests

int error_count=0; //Counts invalid or wrong requests

int byte_count=0; //Counts the number of bytes sent back to the client

char success[1024]; //Buffer holding successful requests as a string

char error[1024]; //Buffer holding wrong requests as a string

char bytes[1024]; //Buffer holding total bytes sent so far


//Function to read file into the buffer word by word
FILE*file;
char* file_read_next()
{
  if (fscanf(file, "%s", content) == EOF) return NULL;
  else return content;
  
}

//The thread which waits for the user input
void*fun1(void*r){
 while(1){
   scanf("%s",user_input);
   if(strcmp(user_input,"q")==0){
    break;
  }
 } 
   int*r1=malloc(sizeof(int));
   pthread_exit(r1);
}


//Server Program
int start_server(int PORT_NUMBER)
{ 
  while(1){
      int i;
      char file_name[1024];
     
      
      // structs to represent the server and client
      struct sockaddr_in server_addr,client_addr;    
      
      int sock; // socket descriptor

      // 1. socket: creates a socket descriptor that you later use to make other system calls
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Socket");
	exit(1);
      }
      int temp;
      if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
	perror("Setsockopt");
	exit(1);
      }

      // configure the server
      server_addr.sin_port = htons(PORT_NUMBER); // specify port number
      server_addr.sin_family = AF_INET;         
      server_addr.sin_addr.s_addr = INADDR_ANY; 
      bzero(&(server_addr.sin_zero),8); 
      
      // 2. bind: use the socket and associate it with the port number
      if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
	perror("Unable to bind");
	exit(1);
      }

      // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
      if (listen(sock, 1) == -1) {
	perror("Listen");
	exit(1);
      }
          
      // once you get here, the server is set up and about to start listening
      printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
      fflush(stdout);
     

      // 4. accept: wait here until we get a connection on that port
      int sin_size = sizeof(struct sockaddr_in);
      int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
      
      // buffer to read data into
      char request[1024];
      
      // 5. recv: read incoming message into buffer
      int bytes_received = recv(fd,request,1024,0);
      // null-terminate the string
      request[bytes_received] = '\0';
      
       for(i=4;request[i]!=' ';i++){
	file_name[i-4]=request[i];
      }
      file_name[i-4]='\0';


    printf("The file name is : %s",file_name);  
    //500 Error 
     if(file_name==NULL || strcmp(file_name,"/favicon.ico")==0){
       printf("\n Invalid file name");
        header="HTTP/1.1 500 Internal Server Error\n\n";
 
       byte_count+= send(fd, header, strlen(header), 0);
       error_message="File name not valid";
       byte_count+= send(fd, error_message, strlen(error_message), 0);  
           
       error_count++;
       close(fd);
       close(sock);
       printf("Server closed connection after null file name\n");
     }
      
    else{  //If statistics requested , send the counts.
           if(strcmp(file_name,"/stats")==0){
            //Send stats 
                         
             sprintf(bytes,"\n Total bytes sent are : %d",byte_count);
             byte_count+= send(fd,bytes, strlen(bytes), 0);
             strcpy(bytes,"");

           
             sprintf(success,"\n No. of successful page requests : %d",success_count);  
             byte_count+=send(fd, success, strlen(success), 0);
             strcpy(success,"");
             
             sprintf(error,"\n No. of unsuccessful page requests : %d",error_count);
             byte_count+= send(fd,error, strlen(error), 0);
             strcpy(error,"");
             
             success_count++;

            close(fd);
            close(sock);
            printf("Server closed connection after stats request\n");

            
          }

        else{ //If valid file exists 
            strcpy(entire_path,path);     
            strcat(entire_path,file_name);     

            file=fopen(entire_path,"r");
            if(file==NULL){
               printf("\n Cannot find the file");
               header="HTTP/1.1 404 Not Found\n\n";
               byte_count+=send(fd, header, strlen(header), 0);
               error_message="Cannot find file ";
               byte_count+= send(fd, error_message, strlen(error_message), 0);       
               error_count++;
               close(fd);
               close(sock);
               printf("Server closed connection after wrong request\n");
       
              }
           else{
               header="HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
               send(fd, header, strlen(header), 0);
               while(file_read_next()!=NULL){
                  byte_count+=send(fd, content, strlen(content), 0);
                  byte_count+=send(fd, " ", strlen(" "), 0);
                }
               success_count++;   
               fclose(file);    
               close(fd);
               close(sock);
               printf("Server closed connection \n");
  
              }//else for file being not null
            }//else for file being not stats
        }//else for file being not invalid name
      if(strcmp(user_input,"q")==0){
        printf("Server accepts one more page request before the it exits\n");
         break;
       }
    }//End  of while loop
  return 0;
} 


int main(int argc, char *argv[])
{
  pthread_t t1;
  void*r;
  // check the number of arguments
  if (argc != 3)
    {
      printf("\nUsage: server [port_number]\n");
      exit(0);
    }
  path=malloc(sizeof(char)*1024);
  if(path==NULL){
    return 1;
 }
  strcpy(path,argv[2]);
  
  pthread_create(&t1,NULL,&fun1,NULL);
 // pthread_join(t1,&r);
  int PORT_NUMBER = atoi(argv[1]);
  
  
  start_server(PORT_NUMBER);
   
  pthread_join(t1,&r);
  free(r);
  free(path);
  return 0;
  // pthread_join(t1,&r);
}

