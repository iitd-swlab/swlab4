#include "sqlite3.h"
#include <iostream>
#include <vector>
#include <string>

int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	if (!argc){
		fprintf(stdout, "Sorry, No Results\n");
	}
	for (i = 0; i<argc; i++){
		fprintf(stdout,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

void exec_query(sqlite3 *db, char *sql, std::string msg){
	char *zErrMsg = 0;
	int status;

	status = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (status != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else{
		msg += "\n";
		fprintf(stdout, msg.c_str());
	}
}


void createTableStudent(sqlite3 *db){
	char *sql = "CREATE TABLE STUDENT("  \
				"ENTRY_NO TEXT PRIMARY KEY	   NOT NULL," \
				"NAME     TEXT                 NOT NULL," \
				"DOB      TEXT				   NOT NULL," \
				"DEPT     TEXT				   NOT NULL," \
				"DEGREE   TEXT				   NOT NULL);";
	exec_query(db, sql, "Created table student");
}

void createTableSubject(sqlite3 *db){
	char *sql = "CREATE TABLE SUBJECT("  \
				"ID			TEXT PRIMARY KEY	NOT NULL," \
				"NAME	    TEXT                NOT NULL," \
				"CREDITS    REAL			    NOT NULL);";
	exec_query(db, sql, "Created table subject");
}

void createTableStLogin(sqlite3 *db){
	char *sql = "CREATE TABLE ST_LOGIN("  \
				"ENTRY_NO   TEXT PRIMARY KEY   NOT NULL," \
				"PASS	    TEXT               NOT NULL);";
	exec_query(db, sql, "Created table st_login");
}

void createTableRegCart(sqlite3 *db){
	char *sql = "CREATE TABLE REG_CART("								    \
				"ENTRY_NO		TEXT				 NOT NULL,"			    \
				"SUBJECT_ID	    TEXT				 NOT NULL,"				\
				"PRIMARY KEY (ENTRY_NO,SUBJECT_ID),"						\
				"FOREIGN KEY(ENTRY_NO) REFERENCES STUDENT(ENTRY_NO),"		\
				"FOREIGN KEY(SUBJECT_ID) REFERENCES SUBJECT(ID) );"			;			
	exec_query(db, sql, "Created table reg_cart");
}

void insert_values(sqlite3 *db, char *table, std::vector<char*> values){
	std::string query = "INSERT INTO ";
	query += table;
	query += " VALUES(";
	for (auto &val: values){
		query += "\"";
		query += val;
		query += "\"";
		query += ',';
	}
	query[query.length() - 1] = ')';
	query += ';';
	std::cout << query << std::endl;
	std::string msg = "inserted into ";
	msg += table;
	exec_query(db, const_cast<char*>(query.c_str()), msg);
}

void search_student(sqlite3 *db){
	std::string entry_no, name, dob, dept, degree;
	std::cout << "Enter entry no. (- to skip):\t";
	std::cin >> entry_no;
	std::cout << "Enter name (- to skip):\t";
	std::cin >> name;
	std::cout << "Enter dob (- to skip):\t";
	std::cin >> dob;
	std::cout << "Enter dept (- to skip):\t";
	std::cin >> dept;
	std::cout << "Enter degree (- to skip):\t";
	std::cin >> degree;
	std::string query = "SELECT * FROM STUDENT WHERE ";
	bool cond = false;

	if (entry_no[0] != '-'){
		query += "ENTRY_NO LIKE '" + entry_no + "'";
		cond = true;
	}
	
	if (name[0] != '-'){
		if (cond){
			query += " AND ";
		}
		query += "NAME LIKE '%" + name + "%'";
		cond = true;
	}
	if (dob[0] != '-'){
		if (cond){
			query += " AND ";
		}
		query += "DOB LIKE '" + dob + "'";
		cond = true;
	}
	if (dept[0] != '-'){
		if (cond){
			query += " AND ";
		}
		query += "DEPT LIKE '" + dept + "'";
		cond = true;
	}
	if (degree[0] != '-'){
		if (cond){
			query += " AND ";
		}
		query += "DEGREE LIKE '" + degree + "'";
	}

	//std::cout << query;
	exec_query(db, const_cast<char*>(query.c_str()), "search complete");
}

void showRegCart(sqlite3 *db){
	std::string entry_no, pass,query;
	std::cout << "Enter entry no:\t";
	std::cin >> entry_no;
	std::cout << "Enter password:\t";
	std::cin >> pass;
	query = "SELECT REG_CART.ENTRY_NO,REG_CART.SUBJECT_ID,SUBJECT.NAME AS 'SUBJECT NAME' FROM REG_CART NATURAL JOIN SUBJECT  NATURAL JOIN ST_LOGIN WHERE ST_LOGIN.ENTRY_NO LIKE '" +\
			entry_no + "' AND ST_LOGIN.PASS LIKE '" + pass + "'";
	exec_query(db, const_cast<char*>(query.c_str()), "");
}

int main(){
	sqlite3 *db;
	char *zErrMsg = 0;
	int status;
	status = sqlite3_open("StudentMgmt.db", &db);
	if (status != SQLITE_OK){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	else{
		fprintf(stdout, "Opened database successfully\n");
	}

	createTableStudent(db);
	createTableSubject(db);
	createTableStLogin(db);
	createTableRegCart(db);
	//insert_values(db,"STUDENT", { "2014EET2822","ARC","2014-01-09","CSE","MTECH" });
	//insert_values(db, "ST_LOGIN", { "2014EET2822", "QAZ" });
	//insert_values(db, "SUBJECT", { "1", "ALGO1", "4.0" });
	//insert_values(db, "REG_CART", { "2014EET2822", "1" });
	//exec_query(db, "select * from student","");
	showRegCart(db);
	search_student(db);
	sqlite3_close(db);

	return 0;
}