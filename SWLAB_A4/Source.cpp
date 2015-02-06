#include "sqlite3.h"
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <algorithm>
#define toUpper(x) std::transform(x.begin(), x.end(), x.begin(), ::toupper)
#define cstr(x) const_cast<char *>(x.c_str())
#define pause() fflush(stdin);std::getchar();break

int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
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

void insert_values(sqlite3 *db, char *table, std::vector<char*> values){
	std::string query = "INSERT INTO ";
	query += table;
	query += " VALUES(";
	for (auto &val : values){
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

void createTableStudent(sqlite3 *db){
	char *sql = "CREATE TABLE STUDENT("  \
				"ENTRY_NO TEXT PRIMARY KEY	   NOT NULL,"		\
				"NAME     TEXT                 NOT NULL,"		\
				"DOB      TEXT				   NOT NULL,"	    \
				"DEPT     TEXT				   NOT NULL,"	    \
				"DEGREE   TEXT				   NOT NULL);"	    ;
	exec_query(db, sql, "Created table student");
}

void createTableSubject(sqlite3 *db){
	char *sql = "CREATE TABLE SUBJECT("  \
				"ID			TEXT PRIMARY KEY	NOT NULL,"				\
				"NAME	    TEXT                NOT NULL,"				\
				"CREDITS    REAL			    CHECK(CREDITS > 0));"	;
	exec_query(db, sql, "Created table subject");
}

void createTableStLogin(sqlite3 *db){
	char *sql = "CREATE TABLE ST_LOGIN("														\
				"ENTRY_NO   TEXT PRIMARY KEY   NOT NULL,"										\
				"PASS	    TEXT               NOT NULL,"										\
				"FOREIGN KEY(ENTRY_NO) REFERENCES STUDENT(ENTRY_NO) ON DELETE CASCADE );"		;
	exec_query(db, sql, "Created table st_login");
}

void createTableTaLogin(sqlite3 *db){
	char *sql = "CREATE TABLE TA_LOGIN("														\
				"ENTRY_NO		TEXT PRIMARY KEY    NOT NULL,"									\
				"PASS	        TEXT				NOT NULL,"									\
				"SUBJECT_ID	    TEXT				NOT NULL,"									\
				"FOREIGN KEY(SUBJECT_ID) REFERENCES SUBJECT(ID) ON DELETE CASCADE );"			;
	exec_query(db, sql, "Created table ta_login");
}

void createTableRegCart(sqlite3 *db){
	char *sql = "CREATE TABLE REG_CART("														 \
				"ENTRY_NO		TEXT				 NOT NULL,"									 \
				"SUBJECT_ID	    TEXT				 NOT NULL,"									 \
				"PRIMARY KEY (ENTRY_NO,SUBJECT_ID),"											 \
				"FOREIGN KEY(ENTRY_NO) REFERENCES STUDENT(ENTRY_NO) ON DELETE CASCADE,"			 \
				"FOREIGN KEY(SUBJECT_ID) REFERENCES SUBJECT(ID) ON DELETE CASCADE );"			 ;			
	exec_query(db, sql, "Created table reg_cart");
}

void createTableMarks(sqlite3 *db){
	char *sql = "CREATE TABLE MARKS("																	\
				"ENTRY_NO		TEXT				 NOT NULL,"											\
				"SUBJECT_ID	    TEXT				 NOT NULL,"											\
				"ASSIGNMENT		TEXT				 NOT NULL,"											\
				"MARKS		    REAL				 CHECK(MARKS > 0),"									\
				"PRIMARY KEY (ENTRY_NO,SUBJECT_ID,ASSIGNMENT),"											\
				"FOREIGN KEY(ENTRY_NO) REFERENCES STUDENT(ENTRY_NO) ON DELETE CASCADE,"					\
				"FOREIGN KEY(SUBJECT_ID) REFERENCES SUBJECT(ID) ON DELETE CASCADE );"					;
	exec_query(db, sql, "Created table marks");
}


void searchStudent(sqlite3 *db){
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

std::pair<bool, std::string> stLogin(sqlite3 *db){
	std::string entry_no, pass, query;
	std::cout << "Enter entry no:\t";
	std::cin >> entry_no;
	std::cout << "Enter password:\t";
	std::cin >> pass;
	auto checkLogin = "SELECT EXISTS(SELECT 1 FROM ST_LOGIN WHERE ENTRY_NO LIKE '" + entry_no + "' AND PASS LIKE '" + pass + "');";
	int status = sqlite3_exec(db, const_cast<char*>(checkLogin.c_str()),
		[](void *NotUsed, int argc, char **argv, char **azColName) -> int
	{
		if (argv[0][0] == '0'){
			std::cout << "Invalid Entry No. / Password Combination" << std::endl;
			return 1;
		}
		return 0;
	}, nullptr, nullptr);
	if (status != SQLITE_OK){
		return std::make_pair(false, "");
	}
	return std::make_pair(true, entry_no);
}

void showRegCart(sqlite3 *db){
	auto loginResult = stLogin(db);
	if(loginResult.first){
		std::cout << "-------- REGISTRATION CART ----------------\n";
		auto query = "SELECT * FROM REG_CART WHERE ENTRY_NO LIKE '" + loginResult.second + "'";
		exec_query(db, const_cast<char*>(query.c_str()), "");
		std::cout << "-------------------------------------------\n";
	}
}

void addSubjectToCart(sqlite3 *db){
	auto loginResult = stLogin(db);
	if (loginResult.first){
		std::cout << "Enter subject id to add: ";
		std::string subjectId;
		std::cin >> subjectId;
		toUpper(loginResult.second);
		insert_values(db, "REG_CART", { const_cast<char*>(loginResult.second.c_str()), const_cast<char*>(subjectId.c_str()) });
	}
}

void delSubjectFromCart(sqlite3 *db){
	auto loginResult = stLogin(db);
	if (loginResult.first){
		std::cout << "Enter subject id to delete: ";
		std::string subjectId;
		std::cin >> subjectId;
		toUpper(loginResult.second);
		auto query = "DELETE FROM REG_CART WHERE ENTRY_NO LIKE '" + loginResult.second + "' AND SUBJECT_ID LIKE '" + subjectId + "';";
		exec_query(db, cstr(query), "Delete successfull");
	}
}

std::tuple<bool, std::string,std::string,std::string> taLogin(sqlite3 *db){
	std::string entry_no, pass, query, subjectId;
	std::cout << "Enter TA entry no:\t";
	std::cin >> entry_no;
	std::cout << "Enter password:\t";
	std::cin >> pass;
	std::cout << "Enter subject id:\t";
	std::cin >> subjectId;
	auto checkLogin = "SELECT EXISTS(SELECT 1 FROM TA_LOGIN WHERE ENTRY_NO LIKE '" + entry_no + "' AND PASS LIKE '" + pass + "' AND SUBJECT_ID LIKE '" + subjectId + "');";
	int status = sqlite3_exec(db, const_cast<char*>(checkLogin.c_str()),
		[](void *NotUsed, int argc, char **argv, char **azColName) -> int
	{
		if (argv[0][0] == '0'){
			std::cout << "Invalid TA Login details" << std::endl;
			return 1;
		}
		return 0;
	}, nullptr, nullptr);
	if (status != SQLITE_OK){
		return std::make_tuple(false, "","","");
	}
	return std::make_tuple(true, entry_no, pass, subjectId);
}

void enterMarks(sqlite3 *db){
	auto loginResult = taLogin(db);
	if (std::get<0>(loginResult)){
		std::string entry_no, assignment,query;
		double marks;
		std::cout << "Enter entry no. of student:\t";
		std::cin >> entry_no;
		std::cout << "Enter assignment id:\t";
		std::cin >> assignment;
		std::cout << "Enter marks:\t";
		std::cin >> marks;
		query = "SELECT EXISTS(SELECT 1 FROM REG_CART WHERE ENTRY_NO LIKE '" + entry_no + "' AND SUBJECT_ID LIKE '" + std::get<3>(loginResult) +"');";

		int status = sqlite3_exec(db, const_cast<char*>(query.c_str()),
			[](void *NotUsed, int argc, char **argv, char **azColName) -> int
		{
			if (argv[0][0] == '0'){
				std::cout << "Invalid Marks Insertion" << std::endl;
				return 1;
			}
			return 0;
		}, nullptr, nullptr);
		if (status != SQLITE_OK){
			return;
		}
		else{
			toUpper(entry_no);
			insert_values(db, "MARKS", { cstr(entry_no), cstr(std::get<3>(loginResult)), cstr(assignment), cstr(std::to_string(marks)) });
		}
	}
}

double total;
void totalMarks(sqlite3 *db){
	auto loginResult = stLogin(db);
	if (loginResult.first){
		std::string subjectId;
		std::cout << "Enter Subject ID: ";
		std::cin >> subjectId;
		total = 0.0;
		auto query = "SELECT MARKS FROM MARKS WHERE ENTRY_NO LIKE'" + loginResult.second + "' AND SUBJECT_ID LIKE '" + subjectId + "';";
		int status = sqlite3_exec(db, const_cast<char*>(query.c_str()),
			[](void *NotUsed, int argc, char **argv, char **azColName) -> int
		{
			for (int i = 0; i<argc; i++){
				//fprintf(stdout, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
				total += std::stod(argv[i]);
			}		
			return 0;
		}, nullptr, nullptr);
		if (status != SQLITE_OK){
			return;
		}
		else{
			std::cout << "Total Marks for Subject ID:" << subjectId << " are " << total << std::endl;
		}
	}
}

void addStudent(sqlite3 *db){
	std::string entry_no, name, dob, dept, degree,pass;
	std::cout << "\nEnter New Student Entry No: ";
	std::cin >> entry_no;
	std::cout << "\nEnter New Student Name: ";
	fflush(stdin);
	std::getline(std::cin, name);
	std::cout << "\nEnter New Student DOB: ";
	std::cin >> dob;
	std::cout << "\nEnter New Student Dept: ";
	std::cin >> dept;
	std::cout << "\nEnter New Student Degree: ";
	std::cin >> degree;
	std::cout << "\nEnter New Student Password: ";
	std::cin >> pass;
	sqlite3_exec(db, "BEGIN", 0, 0, 0);
	toUpper(entry_no);
	toUpper(name);
	insert_values(db, "STUDENT", { cstr(entry_no), cstr(name), cstr(dob), cstr(dept), cstr(degree) });
	insert_values(db, "ST_LOGIN", { cstr(entry_no), cstr(pass) });
	sqlite3_exec(db, "COMMIT", 0, 0, 0);
}

void addSubject(sqlite3 *db){
	std::string id, name;
	double credits;
	std::cout << "\nEnter New Subject ID: ";
	std::cin >> id;
	std::cout << "\nEnter New Subject Name: ";
	std::cin >> name;
	std::cout << "\nEnter New Subject Credits: ";
	std::cin >> credits;
	insert_values(db, "SUBJECT", { cstr(id), cstr(name), cstr(std::to_string(credits)) });
}

void displayMenu(sqlite3 *db){
	for (;;){
		system("cls");
		std::cout <<
			"1. Add a new student\n" \
			"2. Add a new subject\n" \
			"3. Search for a student\n" \
			"4. Show my Registration cart\n" \
			"5. Add Subject to Registration cart\n" \
			"6. Remove Subject from Registration cart\n" \
			"7. Total Marks in a Subject\n" \
			"8. Display a table\n" \
			"----------------- FOR TAs Only ----------------\n" \
			"9. Entry of Marks\n";

		std::string table, query;
		int choice;
		std::cin >> choice;
		switch (choice){
		case 1:addStudent(db); pause();
		case 2:addSubject(db); pause();
		case 3:searchStudent(db); pause();
		case 4:showRegCart(db); pause();
		case 5:addSubjectToCart(db); pause();
		case 6:delSubjectFromCart(db); pause();
		case 7:totalMarks(db); pause();
		case 8:std::cout << "\nEnter table name: ";
			std::cin >> table;
			query = "SELECT * FROM " + table;
			exec_query(db, cstr(query), "");
			pause();
		case 9:enterMarks(db); pause();
		default:exit(0);
		}
	}
}

void seedDatabase(sqlite3 *db){
	exec_query(db, "DROP TABLE IF EXISTS STUDENT", "");
	exec_query(db, "DROP TABLE IF EXISTS SUBJECT", "");
	exec_query(db, "DROP TABLE IF EXISTS REG_CART", "");
	exec_query(db, "DROP TABLE IF EXISTS MARKS", "");
	exec_query(db, "DROP TABLE IF EXISTS ST_LOGIN", "");
	exec_query(db, "DROP TABLE IF EXISTS TA_LOGIN", "");
	createTableStudent(db);
	createTableSubject(db);
	createTableRegCart(db);
	createTableMarks(db);
	createTableStLogin(db);
	createTableTaLogin(db);
	insert_values(db, "STUDENT", { "2014EET2822", "ARC", "2014-01-09", "CSE", "MTECH" });
	insert_values(db, "ST_LOGIN", { "2014EET2822", "QAZ" });
	insert_values(db, "SUBJECT", { "1", "ALGO1", "4.0" });
	insert_values(db, "SUBJECT", { "2", "ALGO2", "4.0" });
	insert_values(db, "REG_CART", { "2014EET2822", "1" });
	insert_values(db, "TA_LOGIN", { "12345", "123","1" });
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


	status = sqlite3_exec(db, " PRAGMA foreign_keys=ON", 0, 0, 0);
	if (status != SQLITE_OK){
		fprintf(stderr, "Foreign key Constriant not enforced: %s\n", sqlite3_errmsg(db));
		exit(0);
	}

	seedDatabase(db);
	std::getchar();
	displayMenu(db);

	sqlite3_close(db);
	return 0;
}