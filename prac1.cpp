#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <jsoncpp/json/json.h>

using namespace std;
namespace fs = std::filesystem;

const int max_String_length = 100; // max dlina stroki

// analog String 
class String {
private:
    char* data;
    size_t size;
public:
    String(const char* str = "") {
        size = strlen(str);
        data = new char[size + 1];
        strcpy(data, str);
    }

    String(const String& other) {
        size = other.size;
        data = new char[size + 1];
        strcpy(data, other.data);
    }

    ~String() {
        delete[] data;
    }

    String& operator=(const String& other) {
        if (this == &other) return *this;
        delete[] data;
        size = other.size;
        data = new char[size + 1];
        strcpy(data, other.data);
        return *this;
    }

    String& operator+(const String& other) {
        size_t newSize = size + other.size;
        char* newData = new char[newSize + 1];
        strcpy(newData, data);
        strcat(newData, other.data);
        delete[] data;
    data = newData;
    size = newSize;
    return *this;
    }

    char& operator[](size_t index) {
        return data[index];
    }

    const char* c_str() const {
        return data;
    }

    size_t length() const {
        return size;
    }

    void clear() {
        size = 0;
        delete[] data;
        data = new char[1];
        data[0] = '\0';
    }
};

//analog vectora
template<typename T>
class Vector {
private:
    T* data;
    size_t capacity;
    size_t size;

    void resize() {
        capacity *= 2;
        T* newData = new T[capacity];
        for (size_t i = 0; i < size; ++i) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }
public:
    Vector() : size(0), capacity(2) {
        data = new T[capacity];
    }

    ~Vector() {
        delete[] data;
    }

    void push_back(const T& value) {
        if (size == capacity) {
            resize();
        }
        data[size++] = value;
    }

    T& operator[](size_t index) {
        return data[index];
    }

    size_t length() const {
        return size;
    }

    void clear() {
        size = 0;
    }
};

// xranenie kolonok tablici
struct tabColumns {
    String name;

    tabColumns() : name("") {}

    tabColumns(const char* colName) : name(colName) {}
};

// xranenie strok tablici
struct tabRows {
    Vector<String> values;
    tabRows() {}
    tabRows(const Vector<String>& vals) : values(vals) {}
};

//class tablici
class table {
public:
    Vector<tabColumns> columns;
    Vector<tabRows> rows;
    String tableName;
    int tuplesLimit;
    int currentFileIndex = 1;
    int primaryKey = 0;

    table() : tableName(""), tuplesLimit(0), currentFileIndex(1) {}

    table(const char* name, int limit) : tableName(name), tuplesLimit(limit) {}

    void addColumn(const char* columnName) { 
        columns.push_back(tabColumns(columnName));
    }

    //proverka, zablokirovana li tablica
    bool isLocked() {
        String lockFilePath = tableName + "_lock";
        return fs::exists(lockFilePath.c_str());
    }

    // ystanovka bloka tablici
    bool lockTable() {
        if (isLocked()) {
            cout << "Таблица " << tableName.c_str() << " уже заблокирована" << endl;
            return false;
        }

        String lockFilePath = tableName + "_lock";
        ofstream lockFile(lockFilePath.c_str());
        lockFile.close();
        return true;
    }

    // snyatie bloka
    void unlockTable() {
        String lockFilePath = tableName + "_lock";
        fs::remove(lockFilePath.c_str());
    }

    void insertRow(const Vector<String>& rowValues) {
        if (!lockTable()) {
            return; // esli tablica v bloke - vstavka ne mojet bit' vipolnena
        }

        if (rowValues.length() != columns.length()) {
            cout << "Ошибка: количество значений не соответствует колчисеству колонок" << endl;
            unlockTable(); // pri owibke razblokiruem tablicy
            return;
        }
        rows.push_back(tabRows(rowValues));
        if (rows.length() >= tuplesLimit) {
            saveToFile();
            rows.clear(); //o4istka zapisei posle soxraneniya
        }

        unlockTable(); //posle yspewnoi vstavki razblokiruem
    }

    void saveToFile() {
        String fileName = tableName;
        fileName = fileName + "/" + to_string(currentFileIndex) + ".csv";
        ofstream file(fileName.c_str(), ios::app);

        if (!file) {
            cout << "Ошибка при открытии файла " << fileName.c_str() << endl;
            return;
        }

        //esli pustoi file - dobavlyaem zagolovki
        if (file.tellp() == 0) {
            for (size_t i = 0; i < columns.length(); ++i) {
                file << columns[i].name.c_str();
                if (i < columns.length() - 1) {
                    file << ",";
                }
            }
            file << endl;
        }

        for (size_t j = 0; j < rows.length(); ++j) {
            for (size_t i = 0; i < rows[j].values.length(); ++i) {
                file << rows[j].values[i].c_str();
                if (i < rows[j].values.length() - 1) {
                    file << ",";
                }
            }
            file << endl;
        }
        file.close();

        // esli dostigli limita strok - pereklyu4aem na next file
        if (rows.length() >= tuplesLimit) {
            currentFileIndex++;
        }
    }

    void createDirectory() {
        fs::create_directory(tableName.c_str());
        ofstream keyFile(tableName.c_str() + String("/primary_key_sequence"), ios::app);
        keyFile.close();
    }

    void loadPrimaryKey() {
        String keyFilePath = tableName + "/primary_key_sequence";
        ifstream keyFile(keyFilePath.c_str());
        if (keyFile.is_open()) {
            keyFile >> primaryKey;
        }
        keyFile.close();
    }

    void savePrimaryKey() {
        String keyFilePath = tableName + "/primary_key_sequence";
        ofstream keyFile(keyFilePath.c_str(), ios::trunc);
        keyFile << primaryKey;
        keyFile.close();
    }

    void displayTable() {
        for (size_t i = 0; i < columns.length(); ++i) {
            cout << columns[i].name.c_str() << "\t";
        }
        cout << endl;
        for (size_t i = 0; i < rows.length(); ++i) {
            for (size_t j = 0; j < rows[i].values.length(); ++j) {
                cout << rows[i].values[j].c_str() << "\t";
            }
            cout << endl;
        }
    }
};

// 4tenie jsona
void readSchemaFromJson(const char* filename, String& dbName, int& tuplesLimit, Vector<table>& tables) {
    ifstream file(filename, ifstream::binary);
    Json::Value root;
    Json::CharReaderBuilder reader;
    String errors;

    if (!Json::parseFromStream(reader, file, &root, &errors)) {
        cerr << "Ошибка чтения json: " << errors << endl;
        return;
    }

    dbName = root["name"].asCString();
    tuplesLimit = root["tuples_limit"].asInt();

    const Json::Value structure = root["structure"];
    for (const auto& tableName : structure.getMemberNames()) {
        table tabl(table.c_str(), tuplesLimit);
        for (const auto& column : structure[tableName]) {
            tabl.addColumn(column.asCString());
        }
        tables.push_back(tabl);
    }
}

// vipolnenie zaprosov kak v sql
void executeSQL(table& tabl, const String& query) {
    char buffer[max_String_length];
    strcpy(buffer, query.c_str());
    char* token = strtok(buffer, " ");

    if (strcmp(token, "INSERT") == 0) {
        strtok(nullptr, " "); // propusk "INTO"
        char* tableName = strtok(nullptr, " ");
        strtok(nullptr, " "); // propusk "VALUES"
        char* valuesStr = strtok(nullptr, "\n");

        Vector<String> values;
        char* value = strtok(valuesStr, ",()");
        while (value != nullptr) {
            values.push_back(String(value));
            value = strtok(nullptr, ",()");
        }
        tabl.insertRow(values);
    } else if (strcmp(token, "DISPLAY") == 0) {
        tabl.displayTable();
    } else {
        cout << "Неизвестная команда.." << endl;
    }
}

int main() {
    String dbName;
    int tuplesLimit;
    Vector<table> tables;

    // 4itaem jsson
    readSchemaFromJson("schema.json", dbName, tuplesLimit, tables);

    // delaem directoriyu
    fs::create_directory(dbName.c_str());
    for (std::size_t i = 0; i < tables.length(); ++i) {
        tables[i].createDirectory()
    }

    cout << "База данных " << dbName.c_str() << " создана" << endl;

    char query[max_String_length];
    while (true) {
        cout << "SQL> ";
        cin.getline(query, max_String_length);
        if (strcmp(query, "exit") == 0) {
            break;
        }

        // poka tol'ko pervaya tablica dlya primera
        /if (tables.length() > 0) {
            executeSQL(tables[0], String(query));
        }
    }

    //soxranenie ostavwixsya strok v faili do zakritiya
    for (size_t i = 0; i < tables.length(); ++i) {
        tables[i].saveToFile();
    }

    return 0;
}
