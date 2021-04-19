#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>


/* OPIS ALGORYTMU

Algorytm polega na znalezieniu najbliższego diamentu (wywołanie BFS) od punktu startowego.
Następnie nowym punktem startowym staje się miejsce po zatrzymaniu, gdy został zebrany najbliży diament.
Algorytm kończy się, gdy wszystkie diamenty zostały zebrane, albo z punktu startowego nie jesteśmy w stanie zebrać pozostałych diamentów.
W drugim przypadku diamenty zebrane podczas ostatniego ruchu zostają zamienione na miny i następuje powrót do oryginalnego punktu startowego.
Po odnalezieniu wszystkich diamentów (tych, które nie zostały zamienione na miny), pola z dodanymi minami są zamieniane na diamenty i 
zostaje wywołany opisany BFS od ostatniego punktu startowego. 
W przypadku znalezienia wszystkich diamentów zostaje zwrócona ścieżka, jeśli nie, oznacza to, 
że istnieją co najmniej dwa korytarze z diamentami bez wyjścia, więc plansza jest nierozwiązywalna i algorytm zwraca "BRAK".
*/


using namespace std;

int diamonds_number = 0;
int collected_diamonds = 0;
struct node* start_node;
int max_moves = 0;
char* input_board;

struct field* maze;
int height,width;

int added_mines = 0;
struct field** added_mines_fields;
struct field** collected_diamonds_in_current_move;
int collected_diamonds_in_current_move_index; 
bool added_mine_in_last_move;




struct path{
    int* path_string;
    int last_index_in_path;
};

struct field{
    int x;
    int y;
    char field_type;
    bool visited;
    struct field* directions[8];
};

struct node{
    struct field* field;
    struct path* path_from_starting_point;
    struct node* prev;
};

struct queue{
    struct node* first;
    struct node* last;
};

struct queue* create_queue(){
    struct queue* out = (struct  queue*) malloc(sizeof(struct queue));
    out->first = nullptr;
    out->last = nullptr;
    return out;
}

struct node* dequeu(struct queue* queue){

    struct node* first_node = queue->first;
    if (queue->first != nullptr) queue->first = queue->first->prev; 
    return first_node;
}

void print_queue(struct queue* queue){
    
    struct node* tmp = queue->first;
    while (tmp != nullptr){
        cout<<"x: "<<tmp->field->x<<" y: "<<tmp->field->y<<"\t";
        tmp = tmp->prev;
    }
    cout<<endl;
}

void enqueue(struct queue* queue, struct node* new_node){
    if (queue->first == nullptr) {
        queue->first = new_node;
        queue->last = new_node;
        new_node->prev = nullptr;
    }
    else {
        queue->last->prev = new_node;
        queue->last = new_node;
        queue->last->prev = nullptr;
    }
}

int is_empty_que(struct queue* queue){
    if (queue->first == nullptr) return true;
    return false;
}


void add_new_direction_to_path(struct path* path, int direction){
    path->path_string[path->last_index_in_path+1] = direction;
    path->last_index_in_path++;
}


void add_two_paths(struct path* path_mother, struct path* path_child){
    for (int i=0; i<= path_child->last_index_in_path; i++){
        add_new_direction_to_path(path_mother, path_child->path_string[i]);
    }
}

struct path* copy_path(struct path* path){
    struct path* copied_path = (struct path*) malloc(sizeof(struct path));
    copied_path->path_string =  (int*) malloc(max_moves * sizeof(int));
    for (int i=0; i< path->last_index_in_path + 1; i++){
        copied_path->path_string[i] = path->path_string[i];
    }
    copied_path->last_index_in_path = path->last_index_in_path;

    return copied_path;    
}


void print_maze(){
    for (int i = 0; i<width*height; i++){
        if (i%width==0) cout<<endl;
        cout<<maze[i].field_type;
    }
    cout<<endl;
}



struct field*  fill_maze(char* input_board){
    struct field* maze = (struct field*) malloc(height * width * sizeof(struct field));
    
    
    for (int i = 0; i<width*height; i++){  
        for(int k=0; k<8; k++){        
            maze[i].directions[k]=nullptr;
        }
    }

                
    for (int i=0; i<(width*height); i++){
        
        maze[i].x = i/width;
        maze[i].y = i-(i/width)*width;
       
       maze[i].field_type = input_board[i];
       if (maze[i].field_type == '.' ) {
           start_node->field = (maze+i);
       }
   
       
       maze[i].visited = false;
       if (maze[i].field_type == '#') continue;
       if (maze[i].field_type == '+') {
           diamonds_number++;  
       }
      

       maze[i+width].directions[0] = &maze[i];                // north 
       maze[i+width-1].directions[1] = &maze[i];              // north-east      
       maze[i-1].directions[2] = &maze[i];                    // east  
       maze[i-width-1].directions[3] = &maze[i];              // south-east        
       maze[i-width].directions[4] = &maze[i];                // south   
       maze[i-width+1].directions[5] = &maze[i];              // south-west 
       maze[i+1].directions[6] = &maze[i];                    // west                
       maze[i+width+1].directions[7] = &maze[i];              // north-west                         
    }
    
    
    added_mines_fields = (struct field**) malloc(diamonds_number * sizeof(struct field*));
    for (int i=0; i<diamonds_number; i++) added_mines_fields[i] = nullptr;
    collected_diamonds_in_current_move = (struct field**) malloc(diamonds_number * sizeof(struct field*));
   
    
    return maze;   
}



struct field* copy_maze(){
    char* board = (char*) malloc(width*height * sizeof(char));
    int prev_diamonds_number = diamonds_number;
    
    for (int i=0; i<width*height; i++) {
        board[i] = input_board[i];
    }
    
    for (int i=0; i<added_mines; i++) {
        int x = added_mines_fields[i]->x;
        int y = added_mines_fields[i]->y;
        board[width * x + y] = '*';    
    }
    
    // copy added mines
    field* added_mines_copy = (struct field*) malloc(diamonds_number * sizeof(struct field));
    for (int i=0; i<added_mines; i++){
        added_mines_copy[i].x = added_mines_fields[i]->x;
        added_mines_copy[i].y = added_mines_fields[i]->y;
    }
    
    struct field* copied_maze = fill_maze(board);
    
    
    for (int i=0; i<added_mines; i++) {
        added_mines_fields[i] = &copied_maze[added_mines_copy[i].x * width + added_mines_copy[i].y];
    }
    diamonds_number = prev_diamonds_number;
    return copied_maze;
}



void find_nearest_diamond(struct path* path){
    
    added_mine_in_last_move = false;
    
    // make every field unvisited
    for (int i = 0; i<width*height; i++) maze[i].visited = false;
    

    struct queue* my_queue =create_queue();
    enqueue(my_queue, start_node);
    
    start_node->path_from_starting_point =  (struct path*) malloc(sizeof(struct path));
    start_node->path_from_starting_point->last_index_in_path = -1;
    start_node->path_from_starting_point->path_string =(int*) malloc(max_moves * (sizeof(int)));

   
  
    
    while (is_empty_que(my_queue) == false){
        struct node* node = dequeu(my_queue);;
        node->field->visited = true;
        
        for (int i=0; i<8; i++){
            struct field* neighb_field = node->field->directions[i];
            if(neighb_field == nullptr) continue;       
            
            if (neighb_field->field_type != '*'){

                //make move 
                 int collected_diam = 0;
                 bool mine_encountered = false;
                 
                 // stone block encountered
                 while (1) {                     
                    if (neighb_field->field_type == '+') {
                        collected_diamonds_in_current_move_index = 0;
                        collected_diam++;
                        
                        // add diamond to collected in current move
                        collected_diamonds_in_current_move[collected_diamonds_in_current_move_index] = neighb_field;
                        collected_diamonds_in_current_move_index++;
                    }
                    
                    
                    else if (neighb_field->field_type == '*') {
                        mine_encountered = true;
                        break;
                    }
                    else if(neighb_field->field_type == 'O') {
                        break;
                    }    
                    if (neighb_field->directions[i] == nullptr) break;                
                    neighb_field = neighb_field->directions[i];  
                 }
                    
                  if (mine_encountered == true) continue;
                  
                  if (collected_diam > 0){

                      struct node* new_node = (struct node*) malloc(sizeof(struct node));
                      new_node->field = neighb_field;
                      new_node->path_from_starting_point = copy_path(node->path_from_starting_point);
                      add_new_direction_to_path(new_node->path_from_starting_point, i);

                      // remove diamonds
                      struct field* tmp = new_node->field;
                      while (tmp != node->field){
                         
                          if (tmp->field_type=='+') {
                              tmp->field_type=' ';
                          }
                          tmp = tmp->directions[(i+4)%8];  
                      }                          
                    
                      collected_diamonds += collected_diam;
                      // set new starting point 
                      start_node = new_node;
                      return;  
                  }
                  else {
                    if (neighb_field->visited == true) continue;
                    
          
                    struct node* new_node = (struct node*) malloc(sizeof(struct node));
                    new_node->field = neighb_field;
                    new_node->path_from_starting_point = copy_path(node->path_from_starting_point);  
                    add_new_direction_to_path(new_node->path_from_starting_point, i);
                    enqueue(my_queue, new_node);    
                }
            }
        }
    }
    
   
    for (int i=0; i<collected_diamonds_in_current_move_index; i++) {
        collected_diamonds_in_current_move[i]->field_type = '*';
    }

    // add to mine array 
    int index = 0;
    while (added_mines_fields[index] != nullptr) index++;

    for (int i=0; i<collected_diamonds_in_current_move_index; i++) {
        added_mines_fields[index+i] = collected_diamonds_in_current_move[i];
        collected_diamonds_in_current_move[i]->field_type = '*';
        added_mines++;
    }  
    added_mine_in_last_move = true;
}


struct path* collect_all_diamonds(){  
    struct path* path = (struct path*) malloc(sizeof(struct path));  
    path->last_index_in_path = -1;
    path->path_string = (int*) malloc(max_moves * (sizeof(int)));
    
    
    while (collected_diamonds < diamonds_number) {       
        find_nearest_diamond(path);
        if (added_mine_in_last_move == false ) {
            ;
        }
        else {
            path->last_index_in_path = -1;
            maze = copy_maze();
        }  
     } 

     maze = copy_maze();

     path->last_index_in_path = -1;
     collected_diamonds = 0;
     while (collected_diamonds < diamonds_number - added_mines) {
        find_nearest_diamond(path);
        if (added_mine_in_last_move == false ) {
            add_two_paths(path, start_node->path_from_starting_point);
        }
     } 
     
     // change mines to diamonds diamonds   
     int mines_number = 0;
     while (added_mines_fields[mines_number] != nullptr) {  
        int x = added_mines_fields[mines_number]->x;
        int y = added_mines_fields[mines_number]->y;
        maze[x*width +y].field_type = '+';        
        mines_number++;
     }


     
     while (collected_diamonds < diamonds_number) {
         find_nearest_diamond(path);
         if (added_mine_in_last_move == false) {
             add_two_paths(path, start_node->path_from_starting_point);
         }
         else throw string("BRAK");
     }
     
    
     return path;
}


int main(){
    start_node = (struct node*) malloc(sizeof(struct node));
    std::cin>>height>>width;

    string dummy;
    getline(cin,dummy);
    string max_moves_str;
    getline(cin,max_moves_str);
    max_moves = std::stoi(max_moves_str) * 3;
    vector<string> str_vec;
    for (int i=0;i<height;i++)
    {
        string tmp_str;
        getline(cin,tmp_str);
        str_vec.push_back(tmp_str);
    }
   
    input_board = new char[height * width +1];
    input_board[height*width]='\0';
    for(int i=0;i<height;i++)
    {
        memcpy(input_board+width*i,str_vec[i].c_str(),width);
    }
    
    


    try {
        maze = fill_maze(input_board);
    }
    catch (string s) { cout<<s<<endl; return(0); }

    try {
        struct path* path = collect_all_diamonds();
        for(int i=0;i<=path->last_index_in_path;i++)
        {
            cout<<path->path_string[i];
        }
    } catch (string s) {  cout<<s; }
    cout<<endl<<flush;
    return(0);
}


