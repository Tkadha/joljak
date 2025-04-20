#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <mysql.h>
#include <iostream>

static bool isValidInput(const char* str) {
	while (*str) {
		if (!isalnum(*str)) { // 영문자와 숫자가 아니면 false
			return false;
		}
		str++;
	}
	return true;
}

class DBManager
{
private:
	MYSQL Conn;             // MySQL 정보를 담을 구조체
	MYSQL* ConnPtr = NULL;  // MySQL 핸들
public:
	DBManager()
	{
		mysql_init(&Conn);      // MySQL 정보 초기화

		ConnPtr = mysql_real_connect(&Conn, "127.0.0.1", "Tkadha", "8923", "survivalodyssey", 3306, (char*)NULL, 0);
		if (ConnPtr == NULL)    // 연결 결과 확인, NULL일 경우 연결 실패한 것
		{
			fprintf(stderr, "MySQL connection error : %s\n", mysql_error(&Conn));
			return;
		}
		// 데이터베이스에 한글이 있다면 아래 3줄 추가
		// MySQL에서 사용하는 문자셋을 VS가 사용하는 문자셋인 euc-kr로 변경해주는 기능
		mysql_query(ConnPtr, "set session character_set_connection=euckr;");
		mysql_query(ConnPtr, "set session character_set_results=euckr;");
		mysql_query(ConnPtr, "set session character_set_client=euckr;");
	}
	~DBManager() { mysql_close(ConnPtr); }

	bool Register(const char* id, const char* pw)
	{
		if (!isValidInput(id) || !isValidInput(pw)) {
			printf("ID 또는 비밀번호에 허용되지 않은 문자가 포함되어 있습니다.\n");
			return false;
		}

		char query[200];
		MYSQL_RES* res; // 쿼리성공시 결과를 담는 구조체 포인터         
		MYSQL_ROW row;	// 쿼리성공시 결과로 나온 행의 정보를 담는 구조체

		// ID 중복 확인 쿼리 실행
		sprintf(query, "SELECT COUNT(*) FROM userinfo WHERE id = '%s'", id);
		if (mysql_query(ConnPtr, query) != 0) {
			fprintf(stderr, "MySQL query error: %s\n", mysql_error(ConnPtr));
			return false;
		}

		res = mysql_store_result(ConnPtr);
		if (res) {
			row = mysql_fetch_row(res);
			int count = atoi(row[0]);  // id 개수 확인
			mysql_free_result(res);

			if (count > 0) {
				printf(" '%s'는 중복된 ID입니다.\n", id);
				return false;
			}
		}
		// 중복이 없을 경우 INSERT 실행
		sprintf(query, "INSERT INTO userinfo (id, password) VALUES ('%s', '%s')", id, pw);
		if (mysql_query(ConnPtr, query) != 0) {
			fprintf(stderr, "MySQL query error: %s\n", mysql_error(ConnPtr));
			return false;
		}
		printf(" '%s' 회원가입 성공!\n", id);
		return true;
	}
	bool Login(const char* id, const char* pw)
	{
		if (!isValidInput(id) || !isValidInput(pw)) {
			printf("ID 또는 비밀번호에 허용되지 않은 문자가 포함되어 있습니다.\n");
			return false;
		}

		char query[200];
		MYSQL_RES* res; // 쿼리성공시 결과를 담는 구조체 포인터         
		MYSQL_ROW row;	// 쿼리성공시 결과로 나온 행의 정보를 담는 구조체

		// ID 와 pw 검사 확인 쿼리 실행
		sprintf(query, "SELECT COUNT(*) FROM userinfo WHERE id = '%s' AND password = '%s'", id, pw);
		if (mysql_query(ConnPtr, query) != 0) {
			fprintf(stderr, "MySQL query error: %s\n", mysql_error(ConnPtr));
			return false;
		}

		res = mysql_store_result(ConnPtr);
		if (!res) {
			fprintf(stderr, "MySQL store result error: %s\n", mysql_error(ConnPtr));
			return false;
		}
		row = mysql_fetch_row(res);
		int count = row ? atoi(row[0]) : 0;  // id 개수 확인
		mysql_free_result(res);

		return count > 0; // 로그인 성공 여부 반환
	}
};

