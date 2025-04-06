#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <mysql.h>
#include <iostream>

static bool isValidInput(const char* str) {
	while (*str) {
		if (!isalnum(*str)) { // �����ڿ� ���ڰ� �ƴϸ� false
			return false;
		}
		str++;
	}
	return true;
}

class DBManager
{
private:
	MYSQL Conn;             // MySQL ������ ���� ����ü
	MYSQL* ConnPtr = NULL;  // MySQL �ڵ�
public:
	DBManager()
	{
		mysql_init(&Conn);      // MySQL ���� �ʱ�ȭ

		ConnPtr = mysql_real_connect(&Conn, "127.0.0.1", "Tkadha", "8923", "survivalodyssey", 3306, (char*)NULL, 0);
		if (ConnPtr == NULL)    // ���� ��� Ȯ��, NULL�� ��� ���� ������ ��
		{
			fprintf(stderr, "MySQL connection error : %s\n", mysql_error(&Conn));
			return;
		}
		// �����ͺ��̽��� �ѱ��� �ִٸ� �Ʒ� 3�� �߰�
		// MySQL���� ����ϴ� ���ڼ��� VS�� ����ϴ� ���ڼ��� euc-kr�� �������ִ� ���
		mysql_query(ConnPtr, "set session character_set_connection=euckr;");
		mysql_query(ConnPtr, "set session character_set_results=euckr;");
		mysql_query(ConnPtr, "set session character_set_client=euckr;");
	}
	~DBManager() { mysql_close(ConnPtr); }

	bool Register(const char* id, const char* pw)
	{
		if (!isValidInput(id) || !isValidInput(pw)) {
			printf("ID �Ǵ� ��й�ȣ�� ������ ���� ���ڰ� ���ԵǾ� �ֽ��ϴ�.\n");
			return false;
		}

		char query[200];
		MYSQL_RES* res; // ���������� ����� ��� ����ü ������         
		MYSQL_ROW row;	// ���������� ����� ���� ���� ������ ��� ����ü

		// ID �ߺ� Ȯ�� ���� ����
		sprintf(query, "SELECT COUNT(*) FROM userinfo WHERE id = '%s'", id);
		if (mysql_query(ConnPtr, query) != 0) {
			fprintf(stderr, "MySQL query error: %s\n", mysql_error(ConnPtr));
			return false;
		}

		res = mysql_store_result(ConnPtr);
		if (res) {
			row = mysql_fetch_row(res);
			int count = atoi(row[0]);  // id ���� Ȯ��
			mysql_free_result(res);

			if (count > 0) {
				printf(" '%s'�� �ߺ��� ID�Դϴ�.\n", id);
				return false;
			}
		}
		// �ߺ��� ���� ��� INSERT ����
		sprintf(query, "INSERT INTO userinfo (id, password) VALUES ('%s', '%s')", id, pw);
		if (mysql_query(ConnPtr, query) != 0) {
			fprintf(stderr, "MySQL query error: %s\n", mysql_error(ConnPtr));
			return false;
		}
		printf(" '%s' ȸ������ ����!\n", id);
		return true;
	}
	bool Login(const char* id, const char* pw)
	{
		if (!isValidInput(id) || !isValidInput(pw)) {
			printf("ID �Ǵ� ��й�ȣ�� ������ ���� ���ڰ� ���ԵǾ� �ֽ��ϴ�.\n");
			return false;
		}

		char query[200];
		MYSQL_RES* res; // ���������� ����� ��� ����ü ������         
		MYSQL_ROW row;	// ���������� ����� ���� ���� ������ ��� ����ü

		// ID �� pw �˻� Ȯ�� ���� ����
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
		int count = row ? atoi(row[0]) : 0;  // id ���� Ȯ��
		mysql_free_result(res);

		return count > 0; // �α��� ���� ���� ��ȯ
	}
};

