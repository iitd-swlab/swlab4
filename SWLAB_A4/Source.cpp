#include "sqlite3.h"
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <algorithm>

#define toUpper(x) std::transform(x.begin(), x.end(), x.begin(), ::toupper)	/**< Transform string from lowercase to uppercase */
#define cstr(x) const_cast<char *>(x.c_str())	/**< Convert std::string to char* */
#define pause() fflush(stdin);std::getchar()	/**< Wait for a keypress */	
#define drawline(x) std::cout<<std::endl;for(auto i=0;i<x;++i) std::cout<<"_";std::cout<<std::endl	/**< Draw a line */	

/**
 * This function is called for each record processed in each SELECT statement executed by sqlite3_exec().
 * @param NotUsed this parameter is used to pass optional data to callback() from sqlite3_exec(). 
 * @param argc the number of columns in the result.
 * @param argv an array of pointers to strings obtained as if from sqlite3_column_text(), one for each column.
 * @param azColName an array of pointers to strings where each entry represents the name of corresponding result column as obtained from sqlite3_column_name().
 * @return 0 if success and non-zero if an error occurs.
 */
int callback(void *NotUsed, int argc, char **argv, char **azColName){
	for (int i = 0; i<argc; i++){
		fprintf(stdout,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

/**
 * This function executes an sql query on the database.
 * @param db a handle to sqlite database.
 * @param sql the sql query to be executed.
 * @param msg a message to print on successful execution of given query.
 */
void exec_query(sqlite3 *db, char *sql, std::string msg){
	char *zErrMsg = 0;
	int status;

	status = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (status != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else if(msg.length()){
		drawline(80);
		fprintf(stdout, msg.c_str());
		drawline(80);
	}
}

/**
 * This function inserts a record with given values into the given table.
 * @param db a handle to sqlite database.
 * @param table the name of the table where the record is to be inserted.
 * @param values a vector of values forming the new record to be inserted.
 */
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
	std::string msg = "Successfull insertion into ";
	msg += table;
	exec_query(db, cstr(query), msg);
}

/**
 * This function creates the table STUDENT.
 * @param db a handle to the sqlite database.
 */
void createTableStudent(sqlite3 *db){
	char *sql = "CREATE TABLE STUDENT("  \
				"ENTRY_NO TEXT PRIMARY KEY	   NOT NULL,"		\
				"NAME     TEXT                 NOT NULL,"		\
				"DOB      TEXT				   NOT NULL,"	    \
				"DEPT     TEXT				   NOT NULL,"	    \
				"DEGREE   TEXT				   NOT NULL);"	    ;
	exec_query(db, sql, "Created table student");
}

/**
 * This function creates the table SUBJECT.
 * @param db a handle to the sqlite database.
 */
void createTableSubject(sqlite3 *db){
	char *sql = "CREATE TABLE SUBJECT("  \
				"ID			TEXT PRIMARY KEY	NOT NULL,"				\
				"NAME	    TEXT                NOT NULL,"				\
				"CREDITS    REAL			    CHECK(CREDITS > 0));"	;
	exec_query(db, sql, "Created table subject");
}

/**
 * This function creates the table ST_LOGIN.
 * @param db a handle to the sqlite database.
 */
void createTableStLogin(sqlite3 *db){
	char *sql = "CREATE TABLE ST_LOGIN("														\
				"ENTRY_NO   TEXT PRIMARY KEY   NOT NULL,"										\
				"PASS	    TEXT               NOT NULL,"										\
				"FOREIGN KEY(ENTRY_NO) REFERENCES STUDENT(ENTRY_NO) ON DELETE CASCADE );"		;
	exec_query(db, sql, "Created table st_login");
}

/**
 * This function creates the table TA_LOGIN.
 * @param db a handle to the sqlite database.
 */
void createTableTaLogin(sqlite3 *db){
	char *sql = "CREATE TABLE TA_LOGIN("														\
				"ENTRY_NO		TEXT PRIMARY KEY    NOT NULL,"									\
				"PASS	        TEXT				NOT NULL,"									\
				"SUBJECT_ID	    TEXT				NOT NULL,"									\
				"FOREIGN KEY(SUBJECT_ID) REFERENCES SUBJECT(ID) ON DELETE CASCADE );"			;
	exec_query(db, sql, "Created table ta_login");
}

/**
 * This function creates the table REG_CART.
 * @param db a handle to the sqlite database.
 */
void createTableRegCart(sqlite3 *db){
	char *sql = "CREATE TABLE REG_CART("														 \
				"ENTRY_NO		TEXT				 NOT NULL,"									 \
				"SUBJECT_ID	    TEXT				 NOT NULL,"									 \
				"PRIMARY KEY (ENTRY_NO,SUBJECT_ID),"											 \
				"FOREIGN KEY(ENTRY_NO) REFERENCES STUDENT(ENTRY_NO) ON DELETE CASCADE,"			 \
				"FOREIGN KEY(SUBJECT_ID) REFERENCES SUBJECT(ID) ON DELETE CASCADE );"			 ;			
	exec_query(db, sql, "Created table reg_cart");
}

/**
 * This function creates the table MARKS.
 * @param db a handle to the sqlite database.
 */
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

/**
 * This function searches for a particular student in the database.
 * @param db a handle to the sqlite database.
 */
void searchStudent(sqlite3 *db){
	std::string entry_no, name, dob, dept, degree;
	std::cout << "\nEnter entry no. (- to skip): ";
	std::cin >> entry_no;
	std::cout << "Enter name (- to skip): ";
	fflush(stdin);
	std::getline(std::cin, name);
	std::cout << "Enter dob (- to skip): ";
	std::cin >> dob;
	std::cout << "Enter dept (- to skip): ";
	std::cin >> dept;
	std::cout << "Enter degree (- to skip): ";
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

	exec_query(db, cstr(query), "..... Search Complete .....");
}

/**
 * This function checks the login credentials of a student.
 * @param db a handle to the sqlite database.
 * @return a tuple of (bool,string) containing the result of login and the entry_no if login successful.
 */
std::pair<bool, std::string> stLogin(sqlite3 *db){
	std::string entry_no, pass, query;
	std::cout << "\nEnter entry no: ";
	std::cin >> entry_no;
	std::cout << "Enter password: ";
	std::cin >> pass;
	auto checkLogin = "SELECT EXISTS(SELECT 1 FROM ST_LOGIN WHERE ENTRY_NO LIKE '" + entry_no + "' AND PASS LIKE '" + pass + "');";
	int status = sqlite3_exec(db, cstr(checkLogin),
		[](void *NotUsed, int argc, char **argv, char **azColName) -> int
	{
		if (argv[0][0] == '0'){
			std::cout << "\nInvalid Entry No. / Password Combination" << std::endl;
			return 1;
		}
		return 0;
	}, nullptr, nullptr);
	if (status != SQLITE_OK){
		return std::make_pair(false, "");
	}
	return std::make_pair(true, entry_no);
}

/**
 * This function shows the Registration Cart of a particular student.
 * @param db a handle to the sqlite database.
 */
void showRegCart(sqlite3 *db){
	auto loginResult = stLogin(db);
	if(loginResult.first){
		auto query = "SELECT * FROM REG_CART WHERE ENTRY_NO LIKE '" + loginResult.second + "'";
		drawline(80);
		exec_query(db, cstr(query), "REGISTRATION CART");
		drawline(80);
	}
}

/**
 * This function adds a Subject to the Registration Cart of a particular student.
 * @param db a handle to the sqlite database.
 */
void addSubjectToCart(sqlite3 *db){
	auto loginResult = stLogin(db);
	if (loginResult.first){
		std::cout << "\nEnter subject id to add: ";
		std::string subjectId;
		std::cin >> subjectId;
		toUpper(loginResult.second);
		insert_values(db, "REG_CART", { cstr(loginResult.second), cstr(subjectId) });
	}
}

/**
 * This function removes a Subject from the Registration Cart of a particular student.
 * @param db a handle to the sqlite database.
 */
void delSubjectFromCart(sqlite3 *db){
	auto loginResult = stLogin(db);
	if (loginResult.first){
		std::cout << "\nEnter subject id to delete: ";
		std::string subjectId;
		std::cin >> subjectId;
		toUpper(loginResult.second);
		auto query = "DELETE FROM REG_CART WHERE ENTRY_NO LIKE '" + loginResult.second + "' AND SUBJECT_ID LIKE '" + subjectId + "';";
		sqlite3_exec(db, "BEGIN", 0, 0, 0);
		exec_query(db, cstr(query), "Subject was successfully removed");
		query = "DELETE FROM MARKS WHERE ENTRY_NO LIKE '" + loginResult.second + "' AND SUBJECT_ID LIKE '" + subjectId + "'; ";
		exec_query(db, cstr(query), "Subject's Marks Deleted");
		sqlite3_exec(db, "COMMIT", 0, 0, 0);
	}
}

/**
 * This function checks the login credentials of a TA.
 * @param db a handle to the sqlite database.
 * @return a tuple of (bool,string,string,string) containing the result of login and entry_no, password & Subject ID if login successful.
 */
std::tuple<bool, std::string,std::string,std::string> taLogin(sqlite3 *db){
	std::string entry_no, pass, query, subjectId;
	std::cout << "\nEnter TA entry no: ";
	std::cin >> entry_no;
	std::cout << "Enter password: ";
	std::cin >> pass;
	std::cout << "Enter subject id: ";
	std::cin >> subjectId;
	auto checkLogin = "SELECT EXISTS(SELECT 1 FROM TA_LOGIN WHERE ENTRY_NO LIKE '" + entry_no + "' AND PASS LIKE '" + pass + "' AND SUBJECT_ID LIKE '" + subjectId + "');";
	int status = sqlite3_exec(db, cstr(checkLogin),
		[](void *NotUsed, int argc, char **argv, char **azColName) -> int
	{
		if (argv[0][0] == '0'){
			std::cout << "\nInvalid TA Login details" << std::endl;
			return 1;
		}
		return 0;
	}, nullptr, nullptr);
	if (status != SQLITE_OK){
		return std::make_tuple(false, "","","");
	}
	return std::make_tuple(true, entry_no, pass, subjectId);
}

/**
 * This function allows the TAs to enter the marks in an Assignment of a Subject taken by a particular Student.
 * @param db a handle to the sqlite database.
 */
void enterMarks(sqlite3 *db){
	auto loginResult = taLogin(db);
	if (std::get<0>(loginResult)){
		std::string entry_no, assignment,query;
		double marks;
		std::cout << "\nEnter entry no. of student: ";
		std::cin >> entry_no;
		std::cout << "Enter assignment id: ";
		std::cin >> assignment;
		std::cout << "Enter marks: ";
		std::cin >> marks;
		query = "SELECT EXISTS(SELECT 1 FROM REG_CART WHERE ENTRY_NO LIKE '" + entry_no + "' AND SUBJECT_ID LIKE '" + std::get<3>(loginResult) +"');";

		int status = sqlite3_exec(db, const_cast<char*>(query.c_str()),
			[](void *NotUsed, int argc, char **argv, char **azColName) -> int
		{
			if (argv[0][0] == '0'){
				std::cout << "\nInvalid Marks Insertion" << std::endl;
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

double total;	/**< Contains value of total marks in a Subject as calculated by totalMarks(). */  

/**
 * This function calculates the total marks in a Subject(Adding all Assignments) taken by a particular student.
 * @param db a handle to the sqlite database.
 */
void totalMarks(sqlite3 *db){
	auto loginResult = stLogin(db);
	if (loginResult.first){
		std::string subjectId;
		std::cout << "Enter Subject ID: ";
		std::cin >> subjectId;
		total = 0.0;
		auto query = "SELECT MARKS FROM MARKS WHERE ENTRY_NO LIKE'" + loginResult.second + "' AND SUBJECT_ID LIKE '" + subjectId + "';";
		int status = sqlite3_exec(db, cstr(query),
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
			std::cout << "\nTotal Marks for Subject ID:" << subjectId << " are " << total << std::endl;
		}
	}
}

/**
 * This function adds a new student to the STUDENT table.
 * @param db a handle to the sqlite database.
 */
void addStudent(sqlite3 *db){
	std::string entry_no, name, dob, dept, degree,pass;
	std::cout << "\nEnter New Student Entry No: ";
	std::cin >> entry_no;
	std::cout << "Enter New Student Name: ";
	fflush(stdin);
	std::getline(std::cin, name);
	std::cout << "Enter New Student DOB: ";
	std::cin >> dob;
	std::cout << "Enter New Student Dept: ";
	std::cin >> dept;
	std::cout << "Enter New Student Degree: ";
	std::cin >> degree;
	std::cout << "Enter New Student Password: ";
	std::cin >> pass;
	toUpper(entry_no);
	toUpper(name);
	sqlite3_exec(db, "BEGIN", 0, 0, 0);
	insert_values(db, "STUDENT", { cstr(entry_no), cstr(name), cstr(dob), cstr(dept), cstr(degree) });
	insert_values(db, "ST_LOGIN", { cstr(entry_no), cstr(pass) });
	sqlite3_exec(db, "COMMIT", 0, 0, 0);
}

/**
* This function adds a new subject to the SUBJECT table.
* @param db a handle to the sqlite database.
*/
void addSubject(sqlite3 *db){
	std::string id, name;
	double credits;
	std::cout << "\nEnter New Subject ID: ";
	std::cin >> id;
	std::cout << "Enter New Subject Name: ";
	std::cin >> name;
	std::cout << "Enter New Subject Credits: ";
	std::cin >> credits;
	insert_values(db, "SUBJECT", { cstr(id), cstr(name), cstr(std::to_string(credits)) });
}

/**
* This function adds a new TA to the TA_LOGIN table.
* @param db a handle to the sqlite database.
*/
void addTa(sqlite3 *db){
	std::string entry_no, pass, subject_id;
	double credits;
	std::cout << "\nEnter New TA Entry No: ";
	std::cin >> entry_no;
	std::cout << "Enter New TA Password: ";
	std::cin >> pass;
	std::cout << "Enter New TA Subject ID: ";
	std::cin >> subject_id;
	toUpper(entry_no);
	insert_values(db, "TA_LOGIN", { cstr(entry_no), cstr(pass), cstr(subject_id) });
}

/**
 * This function displays the program Menu.
 * @param db a handle to the sqlite database.
 */
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
			"9. Execute custom query\n" \
			"----------------- FOR TAs Only ----------------\n" \
			"10. Entry of Marks\n" \
			"11. Add a TA\n";

		std::string table, query;
		int choice;
		std::cin >> choice;
		switch (choice){
		case 1:addStudent(db); pause(); break;
		case 2:addSubject(db); pause(); break;
		case 3:searchStudent(db); pause(); break;
		case 4:showRegCart(db); pause(); break;
		case 5:addSubjectToCart(db); pause(); break;
		case 6:delSubjectFromCart(db); pause(); break;
		case 7:totalMarks(db); pause(); break;
		case 8:
			std::cout << "\nEnter table name: ";
			std::cin >> table;
			query = "SELECT * FROM " + table;
			exec_query(db, cstr(query), "");
			pause(); break;
		case 9:
			std::cout << "\nEnter query: ";
			fflush(stdin);
			std::getline(std::cin, query);
			exec_query(db, cstr(query), "");
			pause(); break;
		case 10:enterMarks(db); pause(); break;
		case 11:addTa(db); pause(); break;
		default:return;
		}
	}
}

/**
 * This function creates all the tables and seeds the database with initial values.
 * @param db a handle to the sqlite database.
 */
void seedDatabase(sqlite3 *db){
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

/**
 * This function creates (opens if exists) the database, seeds it with seedDatabase() and then calls displayMenu() to display the Menu.
 * Finally it closes the database and exits the program.
 */
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