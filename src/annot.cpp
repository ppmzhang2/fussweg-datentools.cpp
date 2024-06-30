#include <csv.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <sstream>
#include <string_view>

#include "annot.hpp"
#include "utils.hpp"

using namespace fdt;

// Anonymous namespace to limit scope to this translation unit
namespace {

    // Manage SQLite3 resources
    // Wrapper classes to handle SQLite3 resources
    class StmtCtx {
      private:
        sqlite3_stmt *stmt_;

      public:
        StmtCtx(sqlite3_stmt *stmt) : stmt_(stmt) {}
        ~StmtCtx() { sqlite3_finalize(stmt_); }
        sqlite3_stmt *get() { return stmt_; }
    };
    // Automatically close the database with unique_ptr
    using DbCtx = std::unique_ptr<sqlite3, decltype(&sqlite3_close)>;

    static constexpr const char *kSqlTransStart = "BEGIN TRANSACTION;";

    static constexpr const char *kSqlCommit = "COMMIT;";

    static constexpr const char *kSqlDropRaw = R"(
        DROP TABLE raw_annot;
        DROP TABLE raw_exif;
    )";

    static constexpr const char *kSqlNew = R"(
        CREATE TABLE raw_exif (
            prefix TEXT,
            image  TEXT,
            height INTEGER,
            width  INTEGER,
            timestamp TEXT
        );
        CREATE TABLE raw_annot (
            prefix TEXT,
            image  TEXT,
            cate   TEXT,
            level  TEXT,
            x      INTEGER,
            y      INTEGER,
            w      INTEGER,
            h      INTEGER
        );

        CREATE TABLE annotations (
            id      INTEGER PRIMARY KEY AUTOINCREMENT,
            img_id  INTEGER,
            cate_id INTEGER,
            x       INTEGER,
            y       INTEGER,
            w       INTEGER,
            h       INTEGER
        );
        CREATE TABLE images (
            id     INTEGER PRIMARY KEY AUTOINCREMENT,
            prefix TEXT,
            image  TEXT,
            height INTEGER,
            width  INTEGER,
            date   TEXT,
            time   TEXT
        );
        CREATE TABLE categories (
            id    INTEGER PRIMARY KEY AUTOINCREMENT,
            cate  TEXT
        );
    )";

    // Import annotation TSV
    static constexpr const char *kSqlImportAnnot =
        "INSERT INTO raw_annot (prefix, image, cate, level, x, y, w, h) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    // Import exif TSV
    static constexpr const char *kSqlImportExif =
        "INSERT INTO raw_exif (prefix, image, height, width, timestamp) "
        "VALUES (?, ?, ?, ?, ?);";

    // Populate categories table with raw annotation data
    static constexpr const char *kSqlFillCategories = R"(
        INSERT INTO categories (cate)
        SELECT DISTINCT cate
          FROM raw_annot
         ORDER BY cate ASC;
    )";

    // Transfer raw EXIF data to the `images` table
    static constexpr const char *kSqlFillImages = R"(
        INSERT INTO images (prefix, image, height, width, date, time)
        SELECT prefix, image, height, width
             , substr(timestamp, 1, 10) AS date
             , substr(timestamp, 12, 8) AS time
          FROM raw_exif
         ORDER BY prefix ASC, image ASC, height ASC, width ASC, date ASC,
                  time ASC;
    )";

    // Populate the annotation table with annotation and image data
    static constexpr const char *kSqlFillAnnot = R"(
        WITH tmp AS (
            SELECT img.id AS img_id
                 , cat.id AS cate_id
                 , ann.x
                 , ann.y
                 , ann.w
                 , ann.h
              FROM raw_annot AS ann
             INNER
              JOIN images AS img
                ON ann.prefix = img.prefix
               AND ann.image = img.image
              JOIN categories AS cat
                ON ann.cate = cat.cate
        )
        INSERT INTO annotations (img_id, cate_id, x, y, w, h)
        SELECT img_id, cate_id, x, y, w, h
          FROM tmp
         ORDER BY img_id ASC, cate_id ASC, x ASC, y ASC, w ASC, h ASC;
    )";

    static constexpr const char *kSqlSelAnnot = R"(
        SELECT id, img_id, cate_id, x, y, w, h
          FROM annotations;
    )";

    static constexpr const char *kSqlSelCate = R"(
        SELECT id, cate FROM categories;
    )";

    static constexpr const char *kSqlSelImg = R"(
        WITH tmp AS (
            SELECT img_id
              FROM annotations
             GROUP BY 1
        )
        SELECT img.id
             , img.prefix
             , img.image
             , img.height
             , img.width
             , img.date
             , img.time
          FROM images AS img
         INNER
          JOIN tmp
            ON img.id= tmp.img_id;
    )";

} // namespace

// Initialize DB
inline static void init_db(sqlite3 **db) {
    int rc = sqlite3_open(":memory:", db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(*db)
                  << std::endl;
        throw std::runtime_error("SQL error");
    }
}

// Function to execute an SQLite statement and handle errors
inline static void exe_stmt(sqlite3 *db, const std::string &sql) {
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL error");
    }
    // Free the error message
    sqlite3_free(errMsg);
}

inline static void bulk_insert_annot(sqlite3 *db, const std::string &tsv) {

    csv::CSVFormat format;
    format.delimiter('\t').header_row(0);
    csv::CSVReader reader(tsv, format);

    // Start a transaction
    exe_stmt(db, kSqlTransStart);

    for (const auto &row : reader) {

        sqlite3_stmt *stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, kSqlImportAnnot, -1, &stmt, nullptr);
        StmtCtx stmt_ctx(std::move(stmt));

        if (rc != SQLITE_OK) {
            std::cerr << "Cannot prepare the INSERTION: " << sqlite3_errmsg(db)
                      << std::endl;
            continue;
        }
        const auto prefix = row["prefix"].get<std::string_view>();
        const auto image = row["image"].get<std::string_view>();
        const auto cate = row["cate"].get<std::string_view>();
        const auto level = row["level"].get<std::string_view>();

        sqlite3_bind_text(stmt_ctx.get(), 1, prefix.data(), prefix.size(),
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt_ctx.get(), 2, image.data(), image.size(),
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt_ctx.get(), 3, cate.data(), cate.size(),
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt_ctx.get(), 4, level.data(), level.size(),
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_ctx.get(), 5, row["x"].get<int>());
        sqlite3_bind_int(stmt_ctx.get(), 6, row["y"].get<int>());
        sqlite3_bind_int(stmt_ctx.get(), 7, row["w"].get<int>());
        sqlite3_bind_int(stmt_ctx.get(), 8, row["h"].get<int>());

        rc = sqlite3_step(stmt_ctx.get());
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to execute the INSERTION: "
                      << sqlite3_errmsg(db) << std::endl;
            continue;
        }
    }

    // Commit the transaction
    exe_stmt(db, kSqlCommit);
}

inline static void bulk_insert_exif(sqlite3 *db, const std::string &tsv) {

    csv::CSVFormat format;
    format.delimiter('\t').header_row(0);
    csv::CSVReader reader(tsv, format);

    // Start a transaction
    exe_stmt(db, kSqlTransStart);

    for (const auto &row : reader) {

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, kSqlImportExif, -1, &stmt, nullptr);
        StmtCtx stmt_ctx(std::move(stmt));

        if (rc != SQLITE_OK) {
            std::cerr << "Cannot prepare the INSERTION: " << sqlite3_errmsg(db)
                      << std::endl;
            continue;
        }
        const auto prefix = row["prefix"].get<std::string_view>();
        const auto image = row["image"].get<std::string_view>();
        const auto ts = row["timestamp"].get<std::string_view>();

        sqlite3_bind_text(stmt_ctx.get(), 1, prefix.data(), prefix.size(),
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt_ctx.get(), 2, image.data(), image.size(),
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_ctx.get(), 3, row["height"].get<int>());
        sqlite3_bind_int(stmt_ctx.get(), 4, row["width"].get<int>());
        sqlite3_bind_text(stmt_ctx.get(), 5, ts.data(), ts.size(),
                          SQLITE_STATIC);

        rc = sqlite3_step(stmt_ctx.get());
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to execute the INSERTION: "
                      << sqlite3_errmsg(db) << std::endl;
            continue;
        }
    }

    // Commit the transaction
    exe_stmt(db, kSqlCommit);
}

// Execute a statement and process the result with a void callback
static void process(sqlite3 *db, const char *sql,
                    void (*callback)(sqlite3_stmt *)) {
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    StmtCtx stmt_ctx(std::move(stmt));

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare the statement: " << sql
                  << " Error: " << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("SQL error");
    }

    rc = sqlite3_step(stmt_ctx.get());
    if (rc != SQLITE_ROW) {
        std::cerr << "Failed to execute the statement: " << sql
                  << " Error: " << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("SQL error");
    }

    while (rc == SQLITE_ROW) {
        callback(stmt_ctx.get());
        rc = sqlite3_step(stmt_ctx.get());
    }
}

inline static void cb_print_int(sqlite3_stmt *stmt) {
    std::cout << "N: " << sqlite3_column_int(stmt, 0) << std::endl;
}

inline static void cb_print_annots(sqlite3_stmt *stmt) {
    std::cout << "|" << sqlite3_column_int(stmt, 0) << "|"
              << sqlite3_column_int(stmt, 1) << "|"
              << sqlite3_column_int(stmt, 2) << "|"
              << sqlite3_column_int(stmt, 3) << "|"
              << sqlite3_column_int(stmt, 4) << "|"
              << sqlite3_column_int(stmt, 5) << "|"
              << sqlite3_column_int(stmt, 6) << "|" << std::endl;
}

inline static void cb_print_exif(sqlite3_stmt *stmt) {
    std::cout << "|" << sqlite3_column_int(stmt, 0) << "|"
              << sqlite3_column_text(stmt, 1) << "|"
              << sqlite3_column_text(stmt, 2) << "|"
              << sqlite3_column_int(stmt, 3) << "|"
              << sqlite3_column_int(stmt, 4) << "|"
              << sqlite3_column_text(stmt, 5) << "|"
              << sqlite3_column_text(stmt, 6) << "|" << std::endl;
}

// Load `categories`, `images`, and `annotations` tables from flat files
static void load_db(sqlite3 *db, const std::string &dir_annot,
                    const std::string &dir_exif) {
    // Load annotation & EXIF TSVs
    for (const auto &f : fdt::utils::listAllFiles(dir_annot, ".tsv")) {
        bulk_insert_annot(db, f);
    }
    for (const auto &f : fdt::utils::listAllFiles(dir_exif, ".tsv")) {
        bulk_insert_exif(db, f);
    }

    // Populate `categories`, `images`, and `annotations` tables
    exe_stmt(db, kSqlFillCategories);
    exe_stmt(db, kSqlFillImages);
    exe_stmt(db, kSqlFillAnnot);
}

inline static nlohmann::json::array_t annot2coco(sqlite3 *db) {
    nlohmann::json::array_t js_annot;

    // Prepare the SQL statement to query the annotations table
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, kSqlSelAnnot, -1, &stmt, nullptr);
    StmtCtx stmt_ctx(std::move(stmt));

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare the SELECT statement: "
                  << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("SQL error");
    }

    // Execute the query and process the results
    while ((rc = sqlite3_step(stmt_ctx.get())) == SQLITE_ROW) {
        // Create the annotation object
        nlohmann::json js;

        const int id = sqlite3_column_int(stmt_ctx.get(), 0);
        const int img_id = sqlite3_column_int(stmt_ctx.get(), 1);
        const int cate_id = sqlite3_column_int(stmt_ctx.get(), 2);
        const int x = sqlite3_column_int(stmt_ctx.get(), 3);
        const int y = sqlite3_column_int(stmt_ctx.get(), 4);
        const int w = sqlite3_column_int(stmt_ctx.get(), 5);
        const int h = sqlite3_column_int(stmt_ctx.get(), 6);

        js["id"] = id;
        js["category_id"] = cate_id;
        js["iscrowd"] = 0;
        js["image_id"] = img_id;
        js["bbox"] = {x, y, w, h};

        // Add the annotation to the JSON array
        js_annot.push_back(js);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute the SELECT statement: "
                  << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("SQL error");
    }

    return js_annot;
}

inline static nlohmann::json::array_t img2coco(sqlite3 *db) {
    nlohmann::json::array_t js_img;

    // Prepare the SQL statement to query the annotations table
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, kSqlSelImg, -1, &stmt, nullptr);
    StmtCtx stmt_ctx(std::move(stmt));

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare the SELECT statement: "
                  << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("SQL error");
    }

    // Execute the query and process the results
    while ((rc = sqlite3_step(stmt_ctx.get())) == SQLITE_ROW) {
        // Create the annotation object
        nlohmann::json js;

        const int id = sqlite3_column_int(stmt_ctx.get(), 0);
        const char *prefix = reinterpret_cast<const char *>(
            sqlite3_column_text(stmt_ctx.get(), 1));
        const char *image = reinterpret_cast<const char *>(
            sqlite3_column_text(stmt_ctx.get(), 2));
        const int height = sqlite3_column_int(stmt_ctx.get(), 3);
        const int width = sqlite3_column_int(stmt_ctx.get(), 4);
        const char *date = reinterpret_cast<const char *>(
            sqlite3_column_text(stmt_ctx.get(), 5));
        const char *time = reinterpret_cast<const char *>(
            sqlite3_column_text(stmt_ctx.get(), 6));

        js["id"] = id;
        js["width"] = width;
        js["height"] = height;
        js["file_name"] = std::string(prefix) + "/" + image;
        js["date_captured"] = date + std::string(" ") + time;

        // Add the annotation to the JSON array
        js_img.push_back(js);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute the SELECT statement: "
                  << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("SQL error");
    }

    return js_img;
}

inline static nlohmann::json::array_t cate2coco(sqlite3 *db) {
    nlohmann::json::array_t js_cate;

    // Prepare the SQL statement to query the annotations table
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, kSqlSelCate, -1, &stmt, nullptr);
    StmtCtx stmt_ctx(std::move(stmt));

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare the SELECT statement: "
                  << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("SQL error");
    }

    // Execute the query and process the results
    while ((rc = sqlite3_step(stmt_ctx.get())) == SQLITE_ROW) {
        // Create the annotation object
        nlohmann::json js;

        const int id = sqlite3_column_int(stmt_ctx.get(), 0);
        const auto cate = reinterpret_cast<const char *>(
            sqlite3_column_text(stmt_ctx.get(), 1));

        js["id"] = id;
        js["name"] = cate;

        // Add the categories to the JSON array
        js_cate.push_back(js);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute the SELECT statement: "
                  << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("SQL error");
    }

    return js_cate;
}

static void db2coco(sqlite3 *db, std::ostream &output_stream) {
    nlohmann::json coco_json;

    coco_json["categories"] = cate2coco(db);
    coco_json["images"] = img2coco(db);
    coco_json["annotations"] = annot2coco(db);

    // Write the JSON object to the output file
    output_stream << coco_json.dump(4) << std::endl;
}

void annot::toCoco(const std::string &dir_annot, const std::string &dir_exif,
                   std::ostream &output_stream) {
    sqlite3 *db = nullptr;
    init_db(&db);
    DbCtx db_ctx(std::move(db), sqlite3_close);

    // Create tables
    exe_stmt(db_ctx.get(), kSqlNew);

    // Load data
    load_db(db_ctx.get(), dir_annot, dir_exif);

    // Drop raw tables
    exe_stmt(db_ctx.get(), kSqlDropRaw);

    // Convert to COCO
    db2coco(db_ctx.get(), output_stream);
}

void annot::print(const std::string &dir_annot, const std::string &dir_exif) {

    sqlite3 *db = nullptr;
    init_db(&db);
    DbCtx db_ctx(std::move(db), sqlite3_close);

    // Create tables
    exe_stmt(db_ctx.get(), kSqlNew);

    // Load data
    load_db(db_ctx.get(), dir_annot, dir_exif);

    // Drop raw tables
    exe_stmt(db_ctx.get(), kSqlDropRaw);

    // Count the #annotations
    process(db_ctx.get(), "SELECT count(1) FROM annotations;", cb_print_int);
    // Print the annotations
    process(db_ctx.get(), kSqlSelAnnot, cb_print_annots);

    // Count the #images
    process(db_ctx.get(), "SELECT count(1) FROM images;", cb_print_int);
    // Print the EXIF data
    process(db_ctx.get(), kSqlSelImg, cb_print_exif);
}
