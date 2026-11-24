from fastapi import FastAPI, HTTPException, Request, Form
from fastapi.responses import HTMLResponse, RedirectResponse,FileResponse
from pydantic import BaseModel
import sqlite3, json, hashlib,pathlib


app = FastAPI(title="Piksel Pals Backend")
DB = "pikselpals.db"
    
def get_db():
    conn = sqlite3.connect(DB)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    with get_db() as db:
        db.executescript("""
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            alias TEXT NOT NULL,
            is_banned INTEGER DEFAULT 0 NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
        );

        CREATE TABLE IF NOT EXISTS progress (
            user_id INTEGER PRIMARY KEY,
            coins INTEGER DEFAULT 0 NOT NULL,
            high_score INTEGER DEFAULT 0 NOT NULL,
            unlocked_stage INTEGER DEFAULT 0 NOT NULL,
            unlocked_chars TEXT DEFAULT '[true, false]' NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS wallets (
            user_id INTEGER PRIMARY KEY,
            balance INTEGER DEFAULT 0 NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS store_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            price INTEGER NOT NULL,
            description TEXT
        );

        CREATE TABLE IF NOT EXISTS inventory (
            user_id INTEGER NOT NULL,
            item_id INTEGER NOT NULL,
            quantity INTEGER DEFAULT 1 NOT NULL,
            PRIMARY KEY(user_id, item_id),
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY(item_id) REFERENCES store_items(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            item_id INTEGER NOT NULL,
            amount INTEGER NOT NULL,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
            status TEXT NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY(item_id) REFERENCES store_items(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS config (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );

        INSERT OR IGNORE INTO store_items (id, name, price, description) VALUES 
            (1, 'Foxy Hero', 15, 'Unlockable quick fox character'),
            (2, 'Mochi Hero', 30, 'Unlockable agile cat character');

        INSERT OR IGNORE INTO config (key, value) VALUES 
            ('gravity', '0.5'),
            ('game_speed', '1.0'),
            ('enemy_spawn_rate', '100'),
            ('maintenance_mode', 'off'),
            ('max_login_attempts', '5'),
            ('min_password_length', '6');
        """)
        
        try:
            db.execute("ALTER TABLE users ADD COLUMN is_banned INTEGER DEFAULT 0 NOT NULL")
            db.commit()
        except sqlite3.OperationalError:
            pass

init_db()

def hash_pw(pw: str) -> str:
    return hashlib.sha256(pw.encode()).hexdigest()

class LoginPayload(BaseModel):
    username: str
    password: str

class RegisterPayload(BaseModel):
    username: str
    password: str
    alias: str = "PixelHero"

class SyncPayload(BaseModel):
    username: str
    coins: int
    score: int
    unlocked_stage: int
    unlocked_chars: list[bool]

def _load_config(db):
    return {r["key"]: r["value"] for r in db.execute("SELECT * FROM config").fetchall()}

def _check_maintenance(cfg):
    if cfg.get("maintenance_mode") == "on":
        raise HTTPException(status_code=503, detail="Server is under maintenance")

def _profile(db, user, cfg):
    prog = db.execute("SELECT * FROM progress WHERE user_id = ?", (user["id"],)).fetchone()
    return {
        "username": user["username"],
        "alias": user["alias"],
        "coins": prog["coins"],
        "high_score": prog["high_score"],
        "unlocked_stage": prog["unlocked_stage"],
        "unlocked_chars": json.loads(prog["unlocked_chars"]),
        "config": cfg,
    }

@app.post("/api/register")
def register(data: RegisterPayload):
    username = data.username.strip()
    alias = data.alias.strip() or "PixelHero"
    if not username:
        raise HTTPException(status_code=400, detail="Enter a username")
    with get_db() as db:
        cfg = _load_config(db)
        _check_maintenance(cfg)
        min_len = int(cfg.get("min_password_length", 6))
        if len(data.password) < min_len:
            raise HTTPException(status_code=400, detail=f"Password needs {min_len}+ characters")
        try:
            cur = db.execute(
                "INSERT INTO users (username, password_hash, alias) VALUES (?, ?, ?)",
                (username, hash_pw(data.password), alias))
        except sqlite3.IntegrityError:
            raise HTTPException(status_code=409, detail="Username already taken")
        db.execute("INSERT INTO progress (user_id) VALUES (?)", (cur.lastrowid,))
        db.commit()
        user = db.execute("SELECT * FROM users WHERE id = ?", (cur.lastrowid,)).fetchone()
        return _profile(db, user, cfg)

@app.post("/api/login")
def login(data: LoginPayload):
    username = data.username.strip()
    with get_db() as db:
        cfg = _load_config(db)
        _check_maintenance(cfg)
        user = db.execute("SELECT * FROM users WHERE username = ?", (username,)).fetchone()
        if not user:
            raise HTTPException(status_code=404, detail="No account with that name")
        if user["password_hash"] != hash_pw(data.password):
            raise HTTPException(status_code=401, detail="Wrong password")
        if user["is_banned"]:
            raise HTTPException(status_code=403, detail="Account suspended")
        return _profile(db, user, cfg)

@app.post("/api/sync")
def sync_progress(data: SyncPayload):
    with get_db() as db:
        user = db.execute("SELECT id FROM users WHERE username = ?", (data.username,)).fetchone()
        if not user:
            raise HTTPException(status_code=404, detail="User not found")
        db.execute("""
            UPDATE progress SET 
                coins = ?, 
                high_score = MAX(high_score, ?), 
                unlocked_stage = MAX(unlocked_stage, ?),
                unlocked_chars = ?
            WHERE user_id = ?
        """, (data.coins, data.score, data.unlocked_stage, json.dumps(data.unlocked_chars), user["id"]))
        db.commit()
    return {"status": "synced"}

# --- Modern Terminal Admin Panel ---
@app.get("/admin", response_class=HTMLResponse)
def admin_dashboard(tab: str = "dashboard"):
    with get_db() as db:
        players = db.execute("""
            SELECT u.id, u.username, u.alias, u.is_banned, p.coins, p.high_score, p.unlocked_stage
            FROM users u JOIN progress p ON u.id = p.user_id
            ORDER BY p.high_score DESC
        """).fetchall()
        configs = db.execute("SELECT * FROM config").fetchall()
        
        total_players = len(players)
        total_coins = sum(p['coins'] for p in players) if players else 0
        top_score = max((p['high_score'] for p in players), default=0)
        banned_count = sum(1 for p in players if p['is_banned'])

    rows = "".join(f"""
        <tr>
            <td style="color: #94a3b8;">#{p['id']}</td>
            <td><strong style="color: #f8fafc;">{p['username']}</strong><br><span style="font-size: 11px; color: #64748b;">{p['alias']}</span></td>
            <td><span style="color: #38bdf8; font-weight: 600;">{p['coins']} COINS</span></td>
            <td style="color: #facc15; font-weight: 600;">{p['high_score']} PTS</td>
            <td>Stage {p['unlocked_stage']+1}</td>
            <td><span class="badge {'badge-banned' if p['is_banned'] else 'badge-active'}">{'SUSPENDED' if p['is_banned'] else 'ACTIVE'}</span></td>
            <td>
                <div style="display: flex; gap: 6px;">
                    <form method="post" action="/admin/grant_coins" style="margin:0;">
                        <input type="hidden" name="user_id" value="{p['id']}">
                        <button type="submit" class="btn-cyan">+50 Coins</button>
                    </form>
                    <form method="post" action="/admin/toggle_ban" style="margin:0;">
                        <input type="hidden" name="user_id" value="{p['id']}">
                        <button type="submit" class="btn-{'danger' if not p['is_banned'] else 'success'}">{'Ban' if not p['is_banned'] else 'Unban'}</button>
                    </form>
                </div>
            </td>
        </tr>
    """ for p in players)

    banned_rows = "".join(f"""
        <tr>
            <td style="color: #94a3b8;">#{p['id']}</td>
            <td><strong style="color: #f8fafc;">{p['username']}</strong></td>
            <td><span class="badge badge-banned">SUSPENDED</span></td>
            <td>
                <form method="post" action="/admin/toggle_ban" style="margin:0;">
                    <input type="hidden" name="user_id" value="{p['id']}">
                    <button type="submit" class="btn-success">Unban Player</button>
                </form>
            </td>
        </tr>
    """ for p in players if p['is_banned'])

    cfg_dict = {c['key']: c['value'] for c in configs}
    
    cfg_inputs = "".join(f"""
        <div class="config-field">
            <label>{c['key'].upper()}</label>
            <input name="{c['key']}" value="{c['value']}">
        </div>
    """ for c in configs if c['key'] in ['gravity', 'game_speed', 'enemy_spawn_rate'])

    security_content = f"""
        <div class="metrics-grid">
            <div class="metric-card">
                <div class="metric-title">Suspended Accounts</div>
                <div class="metric-value" style="color: #f87171;">{banned_count}</div>
            </div>
            <div class="metric-card">
                <div class="metric-title">Maintenance Status</div>
                <div class="metric-value" style="color: {'#f87171' if cfg_dict.get('maintenance_mode')=='on' else '#34d399'};">{cfg_dict.get('maintenance_mode', 'off').upper()}</div>
            </div>
            <div class="metric-card">
                <div class="metric-title">Min Password Length</div>
                <div class="metric-value" style="color: #38bdf8;">{cfg_dict.get('min_password_length', '6')} Chars</div>
            </div>
        </div>

        <div class="panel-section">
            <div class="section-title">🔒 Security & Authentication Policies</div>
            <form method="post" action="/admin/config">
                <div class="config-grid">
                    <div class="config-field">
                        <label>MAINTENANCE MODE (on / off)</label>
                        <input name="maintenance_mode" value="{cfg_dict.get('maintenance_mode', 'off')}">
                    </div>
                    <div class="config-field">
                        <label>MAX LOGIN ATTEMPTS</label>
                        <input name="max_login_attempts" value="{cfg_dict.get('max_login_attempts', '5')}">
                    </div>
                    <div class="config-field">
                        <label>MIN PASSWORD LENGTH</label>
                        <input name="min_password_length" value="{cfg_dict.get('min_password_length', '6')}">
                    </div>
                </div>
                <button type="submit" class="btn-save">Update Security Policies</button>
            </form>
        </div>

        <div class="panel-section">
            <div class="section-title">🚨 Banned / Suspended Accounts Audit</div>
            <table>
                <thead>
                    <tr><th>ID</th><th>Username</th><th>Status</th><th>Actions</th></tr>
                </thead>
                <tbody>
                    {banned_rows if banned_rows else '<tr><td colspan="4" style="text-align: center; color: #64748b; padding: 30px;">No suspended accounts. All players are active!</td></tr>'}
                </tbody>
            </table>
        </div>
    """

    dashboard_content = f"""
        <div class="metrics-grid">
            <div class="metric-card">
                <div class="metric-title">Total Active Players</div>
                <div class="metric-value">{total_players}</div>
            </div>
            <div class="metric-card">
                <div class="metric-title">Circulating Coin Economy</div>
                <div class="metric-value" style="color: #38bdf8;">{total_coins} 🪙</div>
            </div>
            <div class="metric-card">
                <div class="metric-title">Global High Score Peak</div>
                <div class="metric-value" style="color: #facc15;">{top_score} 🏆</div>
            </div>
        </div>

        <div class="panel-section">
            <div class="section-title">⚙️ Live Gameplay Parameters</div>
            <form method="post" action="/admin/config">
                <div class="config-grid">
                    {cfg_inputs}
                </div>
                <button type="submit" class="btn-save">Save Configurations</button>
            </form>
        </div>

        <div class="panel-section">
            <div class="section-title">🏆 Player Leaderboard & Moderation</div>
            <table>
                <thead>
                    <tr><th>ID</th><th>Player / Alias</th><th>Balance</th><th>High Score</th><th>Stage Progress</th><th>Status</th><th>Actions</th></tr>
                </thead>
                <tbody>
                    {rows if rows else '<tr><td colspan="7" style="text-align: center; color: #64748b; padding: 30px;">No players registered yet. Launch tiny2.py to create an account!</td></tr>'}
                </tbody>
            </table>
        </div>
    """

    active_content = security_content if tab == "security" else dashboard_content

    return f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>Piksel Pals // Admin Panel</title>
        <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
        <style>
            * {{ box-sizing: border-box; margin: 0; padding: 0; font-family: 'Inter', sans-serif; }}
            body {{ background-color: #0b0f19; color: #94a3b8; display: flex; height: 100vh; overflow: hidden; }}
            
            .sidebar {{ width: 260px; background-color: #111827; border-right: 1px solid #1f2937; display: flex; flex-direction: column; padding: 24px; }}
            .brand {{ font-size: 16px; font-weight: 700; color: #f8fafc; letter-spacing: 1px; margin-bottom: 32px; display: flex; align-items: center; gap: 10px; }}
            .brand span {{ color: #38bdf8; }}
            .nav-item {{ padding: 12px 16px; border-radius: 8px; color: #94a3b8; font-size: 14px; font-weight: 500; margin-bottom: 6px; cursor: pointer; text-decoration: none; display: block; transition: 0.2s; }}
            .nav-item.active, .nav-item:hover {{ background-color: #1e293b; color: #38bdf8; }}

            .main-content {{ flex: 1; display: flex; flex-direction: column; overflow-y: auto; }}
            .header {{ height: 70px; border-bottom: 1px solid #1f2937; display: flex; align-items: center; justify-content: space-between; padding: 0 32px; background-color: #0b0f19; }}
            .search-bar {{ background: #111827; border: 1px solid #1f2937; border-radius: 8px; padding: 8px 16px; color: #fff; width: 300px; font-size: 13px; }}
            .user-profile {{ display: flex; align-items: center; gap: 12px; color: #f8fafc; font-size: 14px; font-weight: 600; }}
            .avatar {{ width: 36px; height: 36px; border-radius: 50%; background: linear-gradient(135deg, #38bdf8, #818cf8); }}

            .dashboard-body {{ padding: 32px; max-width: 1400px; margin: 0 auto; width: 100%; }}
            
            .metrics-grid {{ display: grid; grid-template-columns: repeat(3, 1fr); gap: 20px; margin-bottom: 32px; }}
            .metric-card {{ background-color: #111827; border: 1px solid #1f2937; border-radius: 12px; padding: 24px; }}
            .metric-title {{ font-size: 12px; font-weight: 600; text-transform: uppercase; color: #64748b; letter-spacing: 0.5px; margin-bottom: 8px; }}
            .metric-value {{ font-size: 28px; font-weight: 700; color: #f8fafc; }}

            .panel-section {{ background-color: #111827; border: 1px solid #1f2937; border-radius: 12px; padding: 24px; margin-bottom: 32px; }}
            .section-title {{ font-size: 16px; font-weight: 600; color: #f8fafc; margin-bottom: 20px; display: flex; justify-content: space-between; align-items: center; }}

            .config-grid {{ display: grid; grid-template-columns: repeat(3, 1fr); gap: 16px; margin-bottom: 16px; }}
            .config-field label {{ display: block; font-size: 11px; font-weight: 600; color: #64748b; margin-bottom: 6px; }}
            .config-field input {{ width: 100%; background: #0b0f19; border: 1px solid #334155; border-radius: 6px; padding: 10px; color: #f8fafc; font-size: 14px; }}
            
            table {{ width: 100%; border-collapse: collapse; text-align: left; }}
            th {{ font-size: 11px; font-weight: 600; text-transform: uppercase; color: #64748b; padding: 12px 16px; border-bottom: 1px solid #1f2937; }}
            td {{ padding: 16px; border-bottom: 1px solid #1f2937; font-size: 13px; color: #cbd5e1; }}
            tr:hover td {{ background-color: rgba(30, 41, 59, 0.4); }}

            .badge {{ padding: 4px 10px; border-radius: 20px; font-size: 11px; font-weight: 600; text-transform: uppercase; }}
            .badge-active {{ background-color: rgba(16, 185, 129, 0.1); color: #34d399; border: 1px solid rgba(16, 185, 129, 0.2); }}
            .badge-banned {{ background-color: rgba(239, 68, 68, 0.1); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.2); }}

            button {{ padding: 6px 12px; border-radius: 6px; font-size: 12px; font-weight: 600; cursor: pointer; border: none; transition: 0.2s; }}
            .btn-cyan {{ background-color: #0284c7; color: #fff; }}
            .btn-cyan:hover {{ background-color: #0369a1; box-shadow: 0 0 12px rgba(56, 189, 248, 0.4); }}
            .btn-danger {{ background-color: #7f1d1d; color: #fca5a5; }}
            .btn-danger:hover {{ background-color: #991b1b; }}
            .btn-success {{ background-color: #065f46; color: #6ee7b7; }}
            .btn-save {{ background: linear-gradient(135deg, #38bdf8, #0284c7); color: #fff; padding: 10px 20px; font-weight: 600; border-radius: 6px; }}
            .btn-save:hover {{ opacity: 0.9; box-shadow: 0 0 15px rgba(56, 189, 248, 0.5); }}
        </style>
    </head>
    <body>
        <div class="sidebar">
            <div class="brand">PIKSELPALS <span>ADMIN panel</span></div>
            <a href="/admin?tab=dashboard" class="nav-item {'active' if tab == 'dashboard' else ''}">📊 Dashboard</a>
            <a href="/admin?tab=dashboard" class="nav-item">🎮 Game Configurations</a>
            <a href="/admin?tab=dashboard" class="nav-item">👥 Player Management</a>
            <a href="/admin?tab=security" class="nav-item {'active' if tab == 'security' else ''}">🔒 Security & Bans</a>
        </div>

        <div class="main-content">
            <div class="header">
                <input type="text" class="search-bar" placeholder="Search assets, players, or transactions...">
                <div class="user-profile">
                    <div class="avatar"></div>
                    Admin Controller
                </div>
            </div>

            <div class="dashboard-body">
                {active_content}
            </div>
        </div>
    </body>
    </html>
    """

@app.post("/admin/grant_coins")
def admin_grant(user_id: int = Form(...)):
    with get_db() as db:
        db.execute("UPDATE progress SET coins = coins + 50 WHERE user_id = ?", (user_id,))
        db.commit()
    return RedirectResponse("/admin?tab=dashboard", status_code=303)

@app.post("/admin/toggle_ban")
def admin_ban(user_id: int = Form(...)):
    with get_db() as db:
        db.execute("UPDATE users SET is_banned = 1 - is_banned WHERE id = ?", (user_id,))
        db.commit()
    return RedirectResponse("/admin?tab=security", status_code=303)

@app.post("/admin/config")
async def admin_config(request: Request):
    form = await request.form()
    with get_db() as db:
        for k, v in form.items():
            db.execute("UPDATE config SET value = ? WHERE key = ?", (str(v), k))
        db.commit()
    return RedirectResponse("/admin?tab=security", status_code=303)

# --- JSON API + live frontend for the admin panel ---
class UserAction(BaseModel):
    user_id: int

class ConfigPayload(BaseModel):
    values: dict[str, str]

@app.get("/panel", include_in_schema=False)
def panel():
    return FileResponse(pathlib.Path(__file__).parent / "admin_panel.html")

@app.get("/api/admin/state")
def admin_state():
    with get_db() as db:
        players = db.execute("""
            SELECT u.id, u.username, u.alias, u.is_banned, p.coins, p.high_score, p.unlocked_stage
            FROM users u JOIN progress p ON u.id = p.user_id
            ORDER BY p.high_score DESC
        """).fetchall()
        configs = db.execute("SELECT key, value FROM config").fetchall()
    return {"players": [dict(p) for p in players],
            "config": {c["key"]: c["value"] for c in configs}}

@app.post("/api/admin/grant_coins")
def api_grant_coins(data: UserAction):
    with get_db() as db:
        cur = db.execute("UPDATE progress SET coins = coins + 50 WHERE user_id = ?", (data.user_id,))
        db.commit()
    if cur.rowcount == 0:
        raise HTTPException(status_code=404, detail="Player not found")
    return {"status": "ok"}

@app.post("/api/admin/toggle_ban")
def api_toggle_ban(data: UserAction):
    with get_db() as db:
        cur = db.execute("UPDATE users SET is_banned = 1 - is_banned WHERE id = ?", (data.user_id,))
        db.commit()
    if cur.rowcount == 0:
        raise HTTPException(status_code=404, detail="Player not found")
    return {"status": "ok"}

@app.post("/api/admin/config")
def api_config(data: ConfigPayload):
    with get_db() as db:
        for k, v in data.values.items():
            db.execute("UPDATE config SET value = ? WHERE key = ?", (v, k))
        db.commit()
    return {"status": "ok"}

# --- Database browser API ---
class CellUpdate(BaseModel):
    table: str
    rowid: int
    column: str
    value: str

class RowRef(BaseModel):
    table: str
    rowid: int

class SqlQuery(BaseModel):
    sql: str

def _tables(db):
    return [r["name"] for r in db.execute(
        "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name")]

def _check_table(db, table):
    if table not in _tables(db):
        raise HTTPException(status_code=404, detail="Unknown table")

def _columns(db, table):
    return [r["name"] for r in db.execute(f'PRAGMA table_info("{table}")')]

@app.get("/api/admin/db/tables")
def db_tables():
    with get_db() as db:
        return [{"name": t, "rows": db.execute(f'SELECT COUNT(*) FROM "{t}"').fetchone()[0]}
                for t in _tables(db)]

@app.get("/api/admin/db/table/{name}")
def db_table(name: str, limit: int = 25, offset: int = 0):
    limit = max(1, min(limit, 100))
    with get_db() as db:
        _check_table(db, name)
        cols = _columns(db, name)
        rows = db.execute(f'SELECT rowid AS __rid, * FROM "{name}" LIMIT ? OFFSET ?',
                          (limit, max(0, offset))).fetchall()
        total = db.execute(f'SELECT COUNT(*) FROM "{name}"').fetchone()[0]
    return {"columns": cols, "rows": [dict(r) for r in rows], "total": total}

@app.post("/api/admin/db/update")
def db_update(data: CellUpdate):
    with get_db() as db:
        _check_table(db, data.table)
        if data.column not in _columns(db, data.table):
            raise HTTPException(status_code=400, detail="Unknown column")
        try:
            cur = db.execute(f'UPDATE "{data.table}" SET "{data.column}" = ? WHERE rowid = ?',
                             (data.value, data.rowid))
        except sqlite3.Error as e:
            raise HTTPException(status_code=400, detail=str(e))
        db.commit()
    if cur.rowcount == 0:
        raise HTTPException(status_code=404, detail="Row not found")
    return {"status": "ok"}

@app.post("/api/admin/db/delete")
def db_delete(data: RowRef):
    with get_db() as db:
        _check_table(db, data.table)
        cur = db.execute(f'DELETE FROM "{data.table}" WHERE rowid = ?', (data.rowid,))
        db.commit()
    if cur.rowcount == 0:
        raise HTTPException(status_code=404, detail="Row not found")
    return {"status": "ok"}

@app.post("/api/admin/db/query")
def db_query(data: SqlQuery):
    if not data.sql.lstrip().lower().startswith(("select", "with")):
        raise HTTPException(status_code=400, detail="Only SELECT queries are allowed")
    with get_db() as db:
        db.execute("PRAGMA query_only = ON")
        try:
            cur = db.execute(data.sql)
            rows = cur.fetchmany(200)
        except sqlite3.Error as e:
            raise HTTPException(status_code=400, detail=str(e))
        cols = [d[0] for d in cur.description] if cur.description else []
    return {"columns": cols, "rows": [list(r) for r in rows]}

# --- Player add / edit API ---
class PlayerNew(BaseModel):
    username: str
    password: str
    alias: str = "PixelHero"
    coins: int = 0
    high_score: int = 0

class PlayerEdit(BaseModel):
    user_id: int
    username: str
    alias: str
    coins: int
    high_score: int

def _validate_player(username, alias, coins, high_score):
    if not username.strip() or not alias.strip():
        raise HTTPException(status_code=400, detail="Username and alias cannot be empty")
    if coins < 0 or high_score < 0:
        raise HTTPException(status_code=400, detail="Coins and high score cannot be negative")

@app.post("/api/admin/player/create")
def api_create_player(data: PlayerNew):
    _validate_player(data.username, data.alias, data.coins, data.high_score)
    with get_db() as db:
        cfg = {r["key"]: r["value"] for r in db.execute("SELECT * FROM config").fetchall()}
        min_len = int(cfg.get("min_password_length", 6))
        if len(data.password) < min_len:
            raise HTTPException(status_code=400, detail=f"Password must be at least {min_len} characters")
        try:
            cur = db.execute(
                "INSERT INTO users (username, password_hash, alias) VALUES (?, ?, ?)",
                (data.username.strip(), hash_pw(data.password), data.alias.strip()))
            db.execute(
                "INSERT INTO progress (user_id, coins, high_score) VALUES (?, ?, ?)",
                (cur.lastrowid, data.coins, data.high_score))
        except sqlite3.IntegrityError:
            raise HTTPException(status_code=409, detail="Username already taken")
        db.commit()
    return {"status": "ok"}

@app.post("/api/admin/player/update")
def api_update_player(data: PlayerEdit):
    _validate_player(data.username, data.alias, data.coins, data.high_score)
    with get_db() as db:
        try:
            cur = db.execute("UPDATE users SET username = ?, alias = ? WHERE id = ?",
                             (data.username.strip(), data.alias.strip(), data.user_id))
            if cur.rowcount == 0:
                raise HTTPException(status_code=404, detail="Player not found")
            db.execute("UPDATE progress SET coins = ?, high_score = ? WHERE user_id = ?",
                       (data.coins, data.high_score, data.user_id))
        except sqlite3.IntegrityError:
            raise HTTPException(status_code=409, detail="Username already taken")
        db.commit()
    return {"status": "ok"}