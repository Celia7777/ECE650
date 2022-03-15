#include <iostream>
#include <pqxx/pqxx>
#include <fstream>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

void executeSQL(string sql, connection *C){
  /* Create a transactional object. */
  work W(*C);
      
  /* Execute SQL query */
  W.exec(sql);
  W.commit();
}

void dropTable(string tablename, connection *C){
  string sql = "DROP TABLE IF EXISTS " + tablename + " CASCADE;";
  executeSQL(sql, C);
}

void createTable(const char* filename, connection *C){
  ifstream file(filename);
  string sqlline;
  string sql;

  if(file.is_open()){
    while(getline(file, sqlline)){
      sql += sqlline;
    }
    file.close();
    executeSQL(sql, C);
    return;
  }
  else{
      cerr<<"Create Table: Unable to open the file!"<<endl;
      return;
  }
}

void addPlayer(const char* filename, connection *C){
  ifstream file(filename);
  int idx;
  int team_id, jersey_num;
  string first_name, last_name;
  int mpg, ppg, rpg, apg;
  double spg, bpg;
  string playerline;

  if(file.is_open()){
    while(getline(file, playerline)){
      stringstream ss;
      ss << playerline;
      ss >> idx >> team_id >> jersey_num >> first_name >> last_name >> mpg >> ppg >> rpg >> apg >> spg >> bpg;
      add_player(C, team_id, jersey_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
    }
    file.close();
    return;
  }
  else{
    cerr<<"Add Player: Unable to open the file!"<<endl;
    return;
  }

}

void addTeam(const char* filename, connection *C){
  ifstream file(filename);
  int idx;
  string name;
  int state_id, color_id, wins, losses;
  string teamline;

  if(file.is_open()){
    while(getline(file, teamline)){
      stringstream ss;
      ss << teamline;
      ss >> idx >> name >> state_id >> color_id >> wins >> losses;
      add_team(C, name, state_id, color_id, wins, losses);
    }
    file.close();
    return;
  }
  else{
    cerr<<"Add Team: Unable to open the file!"<<endl;
    return;
  }
}

void addState(const char* filename, connection *C){
  ifstream file(filename);
  int idx;
  string state;
  string stateline;

  if(file.is_open()){
    while(getline(file, stateline)){
      stringstream ss;
      ss << stateline;
      ss >> idx >> state;
      add_state(C, state);
    }
    file.close();
    return;
  }
  else{
      cerr<<"Add State: Unable to open the file!"<<endl;
      return;
  }  

}

void addColor(const char* filename, connection *C){
  ifstream file(filename);
  string color;
  string colorline;
  int idx;

  if(file.is_open()){
    while(getline(file, colorline)){
      stringstream ss;
      ss << colorline;
      ss >> idx >> color;
      add_color(C, color);
    }
    file.close();
    return;
  }
  else{
      cerr<<"Add Color: Unable to open the file!"<<endl;
      return;
  }  
}



int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection *C;

  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
    
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }


  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files

  dropTable("PLAYER", C);
  dropTable("TEAM", C);
  dropTable("STATE", C);
  dropTable("COLOR", C);
  createTable("createtables.txt", C);
  addState("state.txt", C);
  addColor("color.txt", C);
  addTeam("team.txt", C);
  addPlayer("player.txt", C);


  exercise(C);


  //Close database connection
  C->disconnect();

  return 0;
}


