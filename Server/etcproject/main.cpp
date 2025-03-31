//----------------------------------------------------
// 
// 해당 프로젝트에서 다양한 테스트를 실행한다.
// 
// 3/31 - 데이터베이스 연동 테스트
// 
//----------------------------------------------------
#include <mysql.h>
#include <iostream>

int main()
{
	MYSQL Conn;             // MySQL 정보를 담을 구조체
	MYSQL* ConnPtr = NULL;  // MySQL 핸들
	MYSQL_RES* Result;      // 쿼리성공시 결과를 담는 구조체 포인터
	MYSQL_ROW Row;          // 쿼리성공시 결과로 나온 행의 정보를 담는 구조체
	int Stat;               // 쿼리요청 후 결과(성공, 실패)

	mysql_init(&Conn);      // MySQL 정보 초기화

	// 데이터베이스와 연결
	// ConnPtr = mysql_real_connect(&Conn, "데이터베이스서버주소", "id", "비밀번호", "접속할데이터베이스", 3306, (char*)NULL, 0);
	ConnPtr = mysql_real_connect(&Conn, "127.0.0.1", "Tkadha", "8923", "survivalodyssey", 3306, (char*)NULL, 0);

	if (ConnPtr == NULL)    // 연결 결과 확인, NULL일 경우 연결 실패한 것
	{
		fprintf(stderr, "MySQL connection error : %s\n", mysql_error(&Conn));
		return 1;
	}

	// 데이터베이스에 한글이 있다면 아래 3줄 추가
	// MySQL에서 사용하는 문자셋을 VS가 사용하는 문자셋인 euc-kr로 변경해주는 기능
	mysql_query(ConnPtr, "set session character_set_connection=euckr;");
	mysql_query(ConnPtr, "set session character_set_results=euckr;");
	mysql_query(ConnPtr, "set session character_set_client=euckr;");

	char* Query = (char*)"SELECT * FROM items";    // 세미콜론을 제외한 쿼리문 작성
	Stat = mysql_query(ConnPtr, Query);             // 쿼리 요청 및 성공여부 받아오기
	if (Stat != 0)
	{
		fprintf(stderr, "MySQL query error : %s\n", mysql_error(&Conn));
		return 1;
	}

	Result = mysql_store_result(ConnPtr);           // 결과 확인하기
	while ((Row = mysql_fetch_row(Result)) != NULL) // 결과 출력하기
	{
		std::cout << Row[0] << ' ' << Row[1] << '\n';
	}
	mysql_free_result(Result);                      // 결과 비우기

	// 데이터베이스와 연결해제
	mysql_close(ConnPtr);

	return 0;
}