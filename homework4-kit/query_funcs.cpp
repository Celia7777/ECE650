#include "query_funcs.h"
#include <iomanip>

void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
    work W(*C);
    stringstream ss;
    ss << "INSERT INTO PLAYER (TEAM_ID, UNIFORM_NUM, FIRST_NAME, LAST_NAME, MPG, PPG, RPG, APG, SPG, BPG) VALUES ("<< team_id << ", " << jersey_num << ", " << W.quote(first_name)<<  ", "
        << W.quote(last_name) << ", " << mpg << ", " << ppg << ", " << rpg << ", " << apg << ", " << spg << ", " << bpg << ");";
    W.exec(ss.str());
    W.commit();
}


void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
    work W(*C);
    stringstream ss;
    ss << "INSERT INTO TEAM (NAME, STATE_ID, COLOR_ID, WINS, LOSSES) VALUES (" << "\'"<< name <<"\'" << ", " << state_id << ", " <<color_id<<", "<<wins<<", "<<losses<<");";
    W.exec(ss.str());
    W.commit();
}


void add_state(connection *C, string name)
{
    work W(*C);
    stringstream ss;
    ss << "INSERT INTO STATE (NAME) VALUES ("<<"\'"<< name <<"\'" <<");";
    W.exec(ss.str());
    W.commit();
}


void add_color(connection *C, string name)
{
    work W(*C);
    stringstream ss;
    ss << "INSERT INTO COLOR (NAME) VALUES ("<<"\'"<< name <<"\'" <<");";
    W.exec(ss.str());
    W.commit();
}


// show all attributes of each player with average statistics that fall between 
//the min and max (inclusive) for each enabled statistic
void query1(connection *C,
	    int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
            )
{
    int use_value[6] = {use_mpg, use_ppg, use_rpg, use_apg, use_spg, use_bpg};
    int min_value[4] = {min_mpg, min_ppg, min_rpg, min_apg};
    int max_value[4] = {max_mpg, max_ppg, max_rpg, max_apg};
    double dbmin_value[2] = {min_spg, min_bpg};
    double dbmax_value[2] = {max_spg, max_bpg};

    stringstream ss;
    ss << "SELECT * FROM PLAYER ";
    string select_value[6] = {"MPG","PPG","RPG","APG","SPG","BPG"};
    bool isfirst = true;
    for(int i = 0; i < 6; i++){
        if(use_value[i] == 0){
            continue;
        }
        else{
            if(isfirst){
                ss << "WHERE ";
                isfirst = false;
            }
            else{
                ss << "AND ";
            }
            if(i < 4){
                ss << select_value[i] << " <= " << max_value[i] << " AND " << select_value[i] << " >= " << min_value[i];
            }
            else{
                ss << select_value[i] << " <= " << dbmax_value[i - 4] << " AND " << select_value[i] << " >= " << dbmin_value[i - 4];
            }
        }
    }
    ss << ";";

    /* Create a non-transactional object. */
    nontransaction N(*C);
      
    /* Execute SQL query */
    result R(N.exec(ss.str()));

    cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG"<< endl;
      
    /* List down all the records */
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
      cout << c[0].as<int>() << " " << c[1].as<int>() << " " << c[2].as<int>() << " "<< c[3].as<string>() << " " << c[4].as<string>() << " " << c[5].as<int>() << " "<< c[6].as<int>() << " " << c[7].as<int>() << " " << c[8].as<int>() << " "
        << fixed << setprecision(1) << c[9].as<double>() << " " << c[10].as<double>()<< endl;
    }
}


//show the name of each team with the indicated uniform color
void query2(connection *C, string team_color)
{
    
    stringstream ss;
    ss << "SELECT TEAM.NAME FROM TEAM, COLOR WHERE COLOR.NAME = " << "\'" << team_color <<"\'" <<" AND TEAM.COLOR_ID = COLOR.COLOR_ID;";
    
    nontransaction N(*C);
    result R(N.exec(ss.str()));
    cout << "NAME" << endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        cout << c[0].as<string>() << endl;
    }
}


void query3(connection *C, string team_name)
{
    stringstream ss;
    ss << "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM WHERE TEAM.NAME = "<<"\'"<<team_name<<"\'"<<" AND PLAYER.TEAM_ID = TEAM.TEAM_ID ORDER BY PPG DESC;";

    nontransaction N(*C);
    result R(N.exec(ss.str()));
    cout << "FIRST_NAME LAST_NAME" << endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        cout << c[0].as<string>() << " " << c[1].as<string>() << endl;
    }
}


void query4(connection *C, string team_state, string team_color)
{
    stringstream ss;
    ss << "SELECT PLAYER.FIRST_NAME, PLAYER.LAST_NAME, PLAYER.UNIFORM_NUM FROM PLAYER, STATE, COLOR, TEAM WHERE STATE.NAME = "<<"\'"<<team_state<<"\'"<<" AND COLOR.NAME = "<<"\'"<<team_color<<"\'"<<" AND PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.STATE_ID = STATE.STATE_ID AND TEAM.COLOR_ID = COLOR.COLOR_ID;";

    nontransaction N(*C);
    result R(N.exec(ss.str()));
    cout << "FIRST_NAME LAST_NAME UNIFORM_NUM" << endl; 
    for (result::const_iterator c = R.begin(); c != R.end(); ++c){
        cout << c[0].as<string>() << " " << c[1].as<string>() << " " << c[2].as<int>() <<endl;
    }
}


void query5(connection *C, int num_wins)
{
    stringstream ss;
    ss << "SELECT PLAYER.FIRST_NAME, PLAYER.LAST_NAME, TEAM.NAME, TEAM.WINS FROM PLAYER, TEAM WHERE TEAM.WINS > "<<num_wins<<" AND PLAYER.TEAM_ID = TEAM.TEAM_ID;";
    
    nontransaction N(*C);
    result R(N.exec(ss.str()));
    cout << "FIRST_NAME LAST_NAME NAME WINS" << endl; 
    for (result::const_iterator c = R.begin(); c != R.end(); ++c){
        cout << c[0].as<string>() << " " << c[1].as<string>() <<" "<< c[2].as<string>()<<" "<< c[3].as<int>()<<endl;
    }
}
