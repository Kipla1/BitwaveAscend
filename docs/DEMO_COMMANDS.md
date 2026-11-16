# Demo commands (run from the project root, after building)

## 1. Prove the DBMS connection
    ./build/full/backend/db_smoke
    sqlite3 backend/data/bitwave.db ".tables"
    sqlite3 backend/data/bitwave.db ".schema users"
    sqlite3 -header -column backend/data/bitwave.db "SELECT id, username, alias FROM users;"
    sqlite3 -header -column backend/data/bitwave.db "SELECT * FROM wallets;"
    sqlite3 -header -column backend/data/bitwave.db "SELECT * FROM store_items;"

## 2. Admin panel
    export BITWAVE_ADMIN_PASSWORD=choose-something      # PowerShell: $env:BITWAVE_ADMIN_PASSWORD="choose-something"
    ./build/full/backend/admin_cli

## 3. Run the automated tests (also proves the repositories work)
    ctest --preset full --output-on-failure

## Reset the demo database
    rm backend/data/bitwave.db
