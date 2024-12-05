#include <iostream> 
#include <fstream> 
#include <istream>
#include <sstream> 
#include <filesystem>  
 
using namespace std; 
namespace fs = std::filesystem; 
	
// analog string 
class String { 
    char* data; 
    size_t len; 
	friend istream& operator>>(istream& is, String& str);
	friend istream& getline(istream& is, String&str);
 
public: 
    String() : data(nullptr), len(0) {} 
   
    String(const char* str) { 
        len = 0; 
        while (str[len] != '\0') ++len; 
        data = new char[len + 1]; 
        for (size_t i = 0; i < len; ++i) data[i] = str[i]; 
        data[len] = '\0'; 
    }

    ~String() { delete[] data; } 
   
    String(const String& other) { 
        len = other.len; 
        data = new char[len + 1]; 
        for (size_t i = 0; i < len; ++i) data[i] = other.data[i]; 
        data[len] = '\0'; 
    } 

    String& operator=(const String& other) { 
        if (this != &other) { 
            delete[] data; 
            len = other.len; 
            data = new char[len + 1]; 
            for (size_t i = 0; i < len; ++i) data[i] = other.data[i]; 
            data[len] = '\0'; 
        } 
        return *this; 
    }
	
	friend String operator +(const String& str1, const String str2) {
	    char* new_data = new char[str1.len + str2.len + 1];
	    for (size_t i = 0; i < str1.len; ++i) {
	        new_data[i] = str1.data[i];
	    }
	    
	    for (size_t i = 0; i < str2.len; ++i) {
	        new_data[str1.len+i] = str2.data[i];
	    }
	    
	    new_data[str1.len + str2.len] = '\0';
	    String result(new_data);
	    delete[] new_data;
	    
	    return result;
	}

    void clear() {
        delete[] data;
        data = nullptr;
        len = 0;
    }

    bool operator==(const String& other) const { 
        if (len != other.len) return false; 
        for (size_t i = 0; i < len; ++i) { 
            if (data[i] != other.data[i]) return false; 
        } 
        return true; 
    }

    const char* c_str() const { return data; }

}; 

istream& operator>>(istream& is, String& str) {
    str.clear();
    char buffer[1024];
	is >> buffer;
	str = String(buffer);
	return is;
}

istream& getline(istream& is, String& str) {
	str.clear();
	char buffer[1024];
	is.getline(buffer, sizeof(buffer));
	if (is) {
		str = String(buffer);
	}
	return is;
}
 
// analog vector 
template <typename T> 
class Vector { 
    T* data; 
    size_t capacity; 
    size_t size; 
 
    void resize(size_t new_capacity) { 
        T* new_data = new T[new_capacity]; 
        for (size_t i = 0; i < size; ++i) new_data[i] = data[i]; 
        delete[] data; 
        data = new_data; 
        capacity = new_capacity; 
    } 
 
public: 
    Vector() : data(nullptr), capacity(0), size(0) {} 
    ~Vector() { delete[] data; } 
    void push_back(const T& value) { 
        if (size == capacity) resize(capacity == 0 ? 1 : capacity * 2); 
        data[size++] = value; 
    } 
    T& operator[](size_t index) { return data[index]; } 
    size_t get_size() const { return size; } 
    
    const T* begin() const { return data; }
    const T* end() const { return data + size; }
}; 
 
// bolee prostoi map 
template <typename K, typename V> 
class Map { 
    struct Pair { 
        K key; 
        V value; 
    }; 
 
    Vector<Pair> data; 
 
public: 
    void insert(const K& key, const V& value) { 
        for (size_t i = 0; i < data.get_size(); ++i) { 
            if (data[i].key == key) { 
                data[i].value = value; 
                return; 
            } 
        } 
        data.push_back({key, value}); 
    } 
    V* find(const K& key) { 
        for (size_t i = 0; i < data.get_size(); ++i) { 
            if (data[i].key == key) return &data[i].value; 
        } 
        return nullptr; 
    } 
    const Vector<Pair>& get_data() const { return data; } 
}; 
 
// cfg schemi 
struct SchemaConfig { 
    String name; 
    int tuples_limit; 
    Map<String, Vector<String>> tables; 
}; 
 
// load schemi 
SchemaConfig loadSchema(const char* filename) { 
    ifstream file(filename); 
    if (!file.is_open()) { 
        cerr << "Ошибка: не удалось открыть " << filename << endl; 
        exit(1); 
    } 
 
    SchemaConfig schema; 
    schema.tuples_limit = 1000; // Установка лимита строк по умолчанию 
    schema.name = "Схема"; 
 
    String line; 
    while (getline(file, line)) { 
        Vector<String> columns; 
        istringstream iss(line.c_str()); 
        String tableName; 
        iss >> tableName; 
 
        String column; 
        while (iss >> column) columns.push_back(column); 
 
        schema.tables.insert(tableName, columns); 
    } 
    return schema; 
} 
 
// structura directorii 
void createDirectories(const SchemaConfig& schema) { 
    fs::create_directory(schema.name.c_str()); 
    for (auto& pair : schema.tables.get_data()) { 
        String table = pair.key; 
        String tablePath = schema.name + String("/") + table; 
        fs::create_directory(filesystem::path(tablePath.c_str())); 
 
        ofstream pkFile((tablePath + String("/pk_sequence")).c_str()); 
        pkFile << 1; 
        pkFile.close(); 
 
        ofstream lockFile((tablePath+ String("/lock")).c_str()); 
        lockFile << "UNLOCKED"; 
        lockFile.close(); 
 
        ofstream csvFile((tablePath + String("/1.csv")).c_str()); 
        csvFile.close(); 
    } 
} 
 
// Основной цикл обработки команд 
void processCommand(const String& command, const SchemaConfig& schema) { 
    cout << "Команда: " << command.c_str() << " (обработка не реализована)." << endl; 
} 
 
int main() { 
    // Загрузка схемы 
    SchemaConfig schema = loadSchema("schema.json"); 
    createDirectories(schema); 
 
    // Цикл обработки команд 
    String command; 
    cout << "Введите SQL-запрос или 'EXIT' для выхода:\n"; 
    while (getline(cin, command)) { 
        if (command == "EXIT") break; 
        processCommand(command, schema); 
    } 
    return 0; 
}
