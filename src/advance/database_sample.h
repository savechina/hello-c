#ifndef DATABASE_SAMPLE_H
#define DATABASE_SAMPLE_H

/**
 * @brief Database chapter — SQLite3 C API demo (in-memory)
 *
 * Demonstrates:
 *   1. Open/close in-memory database (":memory:")
 *   2. CREATE TABLE, INSERT, SELECT, UPDATE, DELETE
 *   3. Prepared statements (SQL injection prevention)
 *   4. Error handling (sqlite3_errmsg)
 *   5. Transaction control
 *   6. Column extraction (sqlite3_column_*)
 *
 * Falls back gracefully if sqlite3.h is not available.
 * Called from main_advance() → hello.c → main.c
 */
int main_database_sample(void);

#endif /* DATABASE_SAMPLE_H */
