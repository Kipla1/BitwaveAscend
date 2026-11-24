#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys, math, random, array, os, threading
import requests
import pygame
import sys, math, random, array, os
import pygame
from pygame.locals import *
try:
    import cv2
except ImportError:
    cv2 = None

W, H = 256, 224          # internal "NES-ish" resolution
SCALE = 3                # window = 768 x 672
TS = 16                  # tile size
FPS = 60
SOLID = set("GDB=?")
GROUND_ROW = 11

COL = dict(
    ink=(24, 20, 37), white=(252, 252, 252), black=(0, 0, 0),
    red=(208, 48, 56), dred=(128, 24, 48), orange=(248, 152, 40),
    yellow=(252, 216, 64), pink=(248, 136, 192), grey=(176, 176, 188),
    dgrey=(88, 88, 108), green=(56, 168, 72), blue=(56, 120, 232),
    navy=(16, 16, 48), purple=(136, 88, 208), fire_orange=(255, 100, 0),
    cyan=(0, 220, 252), laser_red=(255, 30, 30),
)
FONTS = {}
CONFIRM = (K_RETURN, K_KP_ENTER, K_SPACE, K_z)
JUMPK = (K_z, K_SPACE, K_UP, K_w)

HERO_DEFS = [
    dict(name='BUNZ', ear='long', body=(120, 216, 140), dark=(56, 148, 88),
         belly=(236, 252, 228), shirt=(232, 72, 72),
         speed=0.95, jump=1.10, hp=3, price=0, bars=(3, 5, 3)),
    dict(name='FOXY', ear='point', body=(248, 152, 40), dark=(200, 96, 24),
         belly=(252, 236, 196), shirt=(56, 120, 232),
         speed=1.15, jump=0.95, hp=3, price=15, bars=(5, 3, 3)),
    dict(name='MOCHI', ear='cat', body=(248, 168, 208), dark=(200, 104, 152),
         belly=(252, 244, 248), shirt=(136, 88, 208),
         speed=1.05, jump=1.02, hp=4, price=30, bars=(4, 4, 4)),
]

THEMES = [
    dict(name='SUNNY MEADOWS', sky=((92, 170, 252), (196, 236, 252)), cloud=(252, 252, 252),
         far=(128, 204, 164), near=(80, 164, 100), grass=(56, 168, 72), lgrass=(136, 216, 96),
         dgrass=(24, 104, 56), dirt=(168, 104, 56), dirt2=(128, 72, 40),
         brick=(224, 128, 56), brick2=(144, 72, 32), night=False, music='stage1'),
    dict(name='SUNSET CANYON', sky=((200, 64, 104), (252, 196, 112)), cloud=(252, 164, 140),
         far=(176, 80, 120), near=(120, 48, 104), grass=(200, 160, 56), lgrass=(248, 216, 96),
         dgrass=(136, 96, 40), dirt=(176, 88, 64), dirt2=(120, 56, 48),
         brick=(200, 96, 72), brick2=(120, 48, 48), night=False, music='stage2'),
    dict(name='MIDNIGHT MOORS', sky=((12, 12, 48), (72, 56, 136)), cloud=(96, 88, 170),
         far=(48, 48, 112), near=(32, 32, 84), grass=(40, 152, 152), lgrass=(104, 216, 200),
         dgrass=(24, 88, 104), dirt=(72, 64, 120), dirt2=(48, 40, 88),
         brick=(112, 88, 176), brick2=(64, 48, 112), night=True, music='stage3'),
]

TIPS = ["VISIT SETTINGS TO LOGIN & TYPE A UNIQUE ALIAS!", "COLLECT COINS TO UNLOCK NEW HEROES!", 
        "HIT ? BLOCKS FOR RANDOM POWER-UPS!", "WATCH OUT FOR LASER-SHOOTING ENEMIES!"]


# ----------------------------------------------------------------- helpers
def lerp(a, b, t):
    return a + (b - a) * t


def lerp_col(c1, c2, t):
    return tuple(int(lerp(c1[i], c2[i], t)) for i in range(3))


def init_fonts():
    FONTS['s'] = pygame.font.Font(None, 16)
    FONTS['m'] = pygame.font.Font(None, 24)
    FONTS['l'] = pygame.font.Font(None, 36)


def draw_text(surf, s, x, y, color=COL['white'], size='s', align='left',
              shadow=True, shadow_col=None, sh=1):
    f = FONTS[size]
    img = f.render(s, False, color)
    r = img.get_rect()
    if align == 'center':
        r.midtop = (x, y)
    elif align == 'right':
        r.topright = (x, y)
    else:
        r.topleft = (x, y)
    if shadow:
        surf.blit(f.render(s, False, shadow_col or COL['ink']), (r.x + sh, r.y + sh))
    surf.blit(img, r)
    return r


def outline(surf, color=COL['ink']):
    w, h = surf.get_size()
    mask = pygame.mask.from_surface(surf)
    sil = mask.to_surface(setcolor=color, unsetcolor=(0, 0, 0, 0))
    out = pygame.Surface((w + 2, h + 2), pygame.SRCALPHA)
    for dx, dy in ((0, 1), (2, 1), (1, 0), (1, 2)):
        out.blit(sil, (dx, dy))
    out.blit(surf, (1, 1))
    return out


NOTE_BASE = {'C': -9, 'D': -7, 'E': -5, 'F': -4, 'G': -2, 'A': 0, 'B': 2}


def note_freq(n):
    if n in ('-', '.'):
        return None
    semis = NOTE_BASE[n[0]] + 12 * (int(n[-1]) - 4) + (1 if '#' in n else 0)
    return 440.0 * 2 ** (semis / 12.0)


# ------------------------------------------------------------------- audio
class Audio:
    def __init__(self):
        self.enabled = False
        self.muted = False
        self.sfx, self.tracks, self.current = {}, {}, None
        self.rate, self.channels = 22050, 1
        try:
            pygame.mixer.quit()
            try:
                pygame.mixer.init(22050, -16, 1, 512, allowedchanges=0)
            except TypeError:
                pygame.mixer.init(22050, -16, 1, 512)
            info = pygame.mixer.get_init()
            if info:
                self.rate, fmt, self.channels = info
                self.enabled = (fmt == -16)
        except pygame.error:
            self.enabled = False

    def tone(self, freq, ms, vol=0.2, duty=0.5, slide=0.0):
        n = int(self.rate * ms / 1000)
        out, ph = array.array('h'), 0.0
        for i in range(n):
            ph = (ph + (freq + slide * i / n) / self.rate) % 1.0
            v = 1 if ph < duty else -1
            out.append(int(v * vol * (1 - i / n) * 32767))
        return out

    def noise(self, ms, vol=0.2):
        n = int(self.rate * ms / 1000)
        return array.array('h', [int(random.uniform(-1, 1) * vol * (1 - i / n) * 32767)
                                 for i in range(n)])

    def _pack(self, samples):
        if self.channels > 1:
            out = array.array('h')
            for s in samples:
                for _ in range(self.channels):
                    out.append(s)
            samples = out
        return pygame.mixer.Sound(buffer=samples.tobytes())

    @staticmethod
    def cat(*parts):
        o = array.array('h')
        for p in parts:
            o.extend(p)
        return o

    def build_sfx(self):
        if not self.enabled:
            return
        T, cat = self.tone, self.cat
        S = self.sfx
        S['jump'] = self._pack(T(300, 170, .18, .25, 420))
        S['coin'] = self._pack(cat(T(988, 70, .16), T(1319, 220, .16)))
        S['stomp'] = self._pack(cat(T(260, 60, .22, .5, -120), self.noise(90, .2)))
        S['powerup'] = self._pack(cat(T(330, 80, .18), T(440, 80, .18), T(554, 80, .18), T(659, 200, .18)))
        S['fireball'] = self._pack(cat(T(600, 50, .15, .5, -300), T(300, 80, .15, .5, -100)))
        S['laser'] = self._pack(cat(T(800, 100, .2, .5, -600)))
        S['punch'] = self._pack(cat(T(150, 80, .25, .5, -80), self.noise(100, .25)))
        S['spinkick'] = self._pack(cat(T(400, 120, .2, .3, 300), T(600, 150, .25, .3, -200)))
        S['hurt'] = self._pack(T(520, 320, .2, .5, -440))
        S['die'] = self._pack(cat(T(494, 110, .2), T(466, 110, .2), T(440, 110, .2),
                                  T(392, 300, .2, .5, -250)))
        S['win'] = self._pack(cat(*[T(f, 120, .16) for f in (523, 659, 784, 1047)],
                                  T(1047, 420, .16, .25)))
        S['move'] = self._pack(T(520, 40, .12))
        S['ok'] = self._pack(cat(T(660, 60, .15), T(990, 140, .15)))
        S['no'] = self._pack(T(160, 160, .18, .5, -40))

    def load_external_mp3s(self):
        """Loads mp3_1.mp3 as lobby track and mp3_9.mp3 as level 2 track if present."""
        if not self.enabled:
            return
        try:
            if os.path.exists('01 ...Press Start Already! (Title Screen).mp3'):
                self.tracks['lobby'] = pygame.mixer.Sound('01 ...Press Start Already! (Title Screen).mp3')
                self.tracks['lobby'].set_volume(0.6)
        except Exception:
            pass
        if not self.enabled:
            return
        
        try:
            if os.path.exists('03 Chiro & the Autumn Winds (Forest Stage).mp3'):
                self.tracks['stage1'] = pygame.mixer.Sound('03 Chiro & the Autumn Winds (Forest Stage).mp3')
                self.tracks['stage1'].set_volume(0.6)
        except Exception:
            pass
        
        try:
            if os.path.exists('09 Behold, the SSSSS-Class Idol Makes a Splash!! ~ Seductively Spellbound Scandalous Squid-tastic Superstar Idol! (Squid Boss).mp3'):
                self.tracks['stage2'] = pygame.mixer.Sound('09 Behold, the SSSSS-Class Idol Makes a Splash!! ~ Seductively Spellbound Scandalous Squid-tastic Superstar Idol! (Squid Boss).mp3')
                self.tracks['stage2'].set_volume(0.6)
        except Exception:
            pass

        try:
            if os.path.exists('11 As You Are, Forever Blue Skies (City Stage).mp3'):
                self.tracks['stage3'] = pygame.mixer.Sound('11 As You Are, Forever Blue Skies (City Stage).mp3')
                self.tracks['stage3'].set_volume(0.6)
        except Exception:
            pass

    def build_track(self, name, lead, bass, step_ms, lvol=.15, bvol=.11):
        if not self.enabled or name in self.tracks:
            return
        L, B = lead.split(), bass.split()
        spm = int(self.rate * step_ms / 1000)
        out = array.array('h')
        for i in range(len(L)):
            lf, bf = note_freq(L[i]), note_freq(B[i % len(B)])
            lph = bph = 0.0
            for k in range(spm):
                v = 0.0
                if lf and k < spm * 0.85:
                    lph = (lph + lf / self.rate) % 1.0
                    v += (1 if lph < 0.25 else -1) * lvol * (1 - 0.5 * k / spm)
                if bf:
                    bph = (bph + bf / self.rate) % 1.0
                    v += (1 if bph < 0.5 else -1) * bvol * (1 - 0.3 * k / spm)
                out.append(int(v * 32767))
        snd = self._pack(out)
        snd.set_volume(0.55)
        self.tracks[name] = snd

    def build_lobby(self):
        self.build_track(
            'lobby',
            "E5 - G5 - E5 - C5 - D5 - F5 - D5 - B4 - C5 - E5 - G5 - A5 - G5 - E5 - D5 - "
            "E5 - G5 - E5 - C5 - D5 - F5 - A5 - F5 - G5 - E5 - C5 - D5 - C5 - - -",
            "C3 - G3 - C3 - G3 - F3 - C3 - F3 - C3 - C3 - G3 - C3 - G3 - G3 - D3 - G3 - D3 -", 150)

    def build_game(self):
        self.build_track(
            'game',
            "C5 C5 - E5 - G5 - E5 C5 - D5 - F5 - A4 - G4 B4 D5 - G5 - D5 B4 - C5 - E5 - G5 - "
            "A5 A5 - G5 - E5 - C5 A4 - B4 - D5 - G5 - E5 - D5 - C5 - - - E5 - D5 - C5 - -",
            "C3 - C4 - C3 - C4 - F3 - F4 - F3 - F4 - G3 - G4 - G3 - G4 - C3 - C4 - C3 - C4 -", 120)
        self.build_track(
            'stage2',
            "G5 E5 C5 E5 G5 - B5 A5 G5 E5 - C5 D5 E5 F5 G5 -",
            "G3 - C3 - G3 - C3 - F3 - G3 - C3 - G3 -", 130)

    def play(self, name):
        if self.enabled and name in self.sfx and not self.muted:
            self.sfx[name].play()

    def play_music(self, name):
        if not self.enabled or self.current == name:
            return
        self.stop_music()
        self.current = name
        if name in self.tracks and not self.muted:
            self.tracks[name].play(-1)

    def stop_music(self):
        if self.enabled:
            for t in self.tracks.values():
                t.stop()
        self.current = None

    def toggle_mute(self):
        self.muted = not self.muted
        if not self.enabled:
            return
        if self.muted:
            for t in self.tracks.values():
                t.stop()
        elif self.current in self.tracks:
            self.tracks[self.current].play(-1)


# --------------------------------------------------------------------- art
class Art:
    def __init__(self):
        self.heroes, self.enemy, self.tiles, self.bgs = {}, {}, [], []
        self.stars, self.hearts, self.ui = [], {}, {}

        try:
            self.custom_bg = pygame.image.load('skyline-b.png').convert()
            self.custom_bg = pygame.transform.scale(self.custom_bg, (W, H))
        except Exception:
            self.custom_bg = None

    def build_heroes(self):
        for k, d in enumerate(HERO_DEFS):
            fr = {}
            for pose in ('idle', 'run0', 'run1', 'run2', 'jump', 'punch', 'kick'):
                s = self._hero_frame(d, pose)
                fr[(pose, 1)] = s
                fr[(pose, -1)] = pygame.transform.flip(s, True, False)
            self.heroes[k] = fr

    def _hero_frame(self, d, pose):
        s = pygame.Surface((20, 20), pygame.SRCALPHA)
        body, dark, belly, shirt = d['body'], d['dark'], d['belly'], d['shirt']
        R, P = pygame.draw.rect, pygame.draw.polygon
        ox = 2 if pose in ('punch', 'kick') else 0
        
        if d['ear'] == 'point':
            R(s, body, (ox + 0, 13, 4, 4)); R(s, COL['white'], (ox + 0, 13, 2, 2))
            P(s, body, [(ox + 3, 6), (ox + 4, 1), (ox + 8, 6)]); P(s, body, [(ox + 8, 6), (ox + 12, 1), (ox + 13, 6)])
            R(s, dark, (ox + 4, 3, 1, 3)); R(s, dark, (ox + 11, 3, 1, 3))
        elif d['ear'] == 'long':
            for ex in (ox + 4, ox + 9):
                R(s, body, (ex, 0, 3, 7)); R(s, COL['pink'], (ex + 1, 1, 1, 5))
        else:
            P(s, body, [(ox + 3, 6), (ox + 3, 2), (ox + 8, 5)]); P(s, body, [(ox + 8, 5), (ox + 13, 2), (ox + 13, 6)])
            R(s, COL['pink'], (ox + 4, 4, 1, 2)); R(s, COL['pink'], (ox + 11, 4, 1, 2))
            
        R(s, body, (ox + 3, 5, 10, 8))
        R(s, COL['white'], (ox + 5, 7, 3, 3)); R(s, COL['white'], (ox + 9, 7, 3, 3))
        R(s, COL['ink'], (ox + 7, 7, 1, 3)); R(s, COL['ink'], (ox + 11, 7, 1, 3))
        R(s, belly, (ox + 7, 10, 6, 3)); R(s, COL['ink'], (ox + 12, 10, 1, 1)); R(s, dark, (ox + 8, 12, 3, 1))
        R(s, shirt, (ox + 4, 13, 8, 4)); R(s, belly, (ox + 6, 13, 4, 1))
        
        if pose == 'punch':
            R(s, shirt, (ox + 12, 13, 6, 3))
            R(s, body, (ox + 16, 13, 3, 3))
        elif pose == 'kick':
            R(s, dark, (ox + 12, 15, 6, 3))
        else:
            R(s, body, (ox + 2, 14, 2, 3)); R(s, body, (ox + 12, 14, 2, 3))
            
        legs = {'idle': [(ox + 4, 17, 3, 3), (ox + 9, 17, 3, 3)],
                'run0': [(ox + 3, 17, 4, 3), (ox + 9, 16, 3, 3)],
                'run1': [(ox + 5, 17, 3, 3), (ox + 8, 17, 3, 3)],
                'run2': [(ox + 9, 17, 4, 3), (ox + 4, 16, 3, 3)],
                'jump': [(ox + 3, 16, 4, 3), (ox + 10, 15, 3, 3)],
                'punch': [(ox + 4, 17, 3, 3), (ox + 9, 17, 3, 3)],
                'kick': [(ox + 6, 16, 8, 3)]}[pose]
        for r in legs:
            R(s, dark, r)
        return outline(s)

    def build_enemy(self):
        for f in (0, 1):
            s = pygame.Surface((16, 16), pygame.SRCALPHA)
            body, dark = (200, 72, 72), (144, 40, 40)
            pygame.draw.ellipse(s, body, (0, 3, 16, 12))
            for x in (3, 8):
                pygame.draw.polygon(s, dark, [(x, 5), (x + 2, 0), (x + 5, 5)])
            pygame.draw.rect(s, COL['white'], (3, 6, 4, 4)); pygame.draw.rect(s, COL['white'], (9, 6, 4, 4))
            pygame.draw.rect(s, COL['ink'], (5, 7, 2, 3)); pygame.draw.rect(s, COL['ink'], (9, 7, 2, 3))
            pygame.draw.line(s, COL['ink'], (3, 5), (7, 7)); pygame.draw.line(s, COL['ink'], (12, 5), (9, 7))
            pygame.draw.rect(s, COL['ink'], (6, 11, 4, 2)); pygame.draw.rect(s, COL['white'], (7, 11, 1, 1))
            if f == 0:
                pygame.draw.rect(s, dark, (1, 13, 5, 3)); pygame.draw.rect(s, dark, (10, 13, 5, 3))
            else:
                pygame.draw.rect(s, dark, (3, 13, 5, 3)); pygame.draw.rect(s, dark, (8, 13, 5, 3))
            self.enemy[f] = outline(s)

    def build_ui(self):
        heart = ["..XX.XX..", ".XXXXXXX.", "XXXXXXXXX", "XXXXXXXXX", ".XXXXXXX.", "..XXXXX..", "...XXX...", "....X...."]
        self.hearts['full'] = outline(from_map(heart, {'X': COL['red']}))
        self.hearts['empty'] = outline(from_map(heart, {'X': (72, 56, 88)}))
        for i in range(8):
            ang = i / 8 * math.pi
            s = pygame.Surface((12, 12), pygame.SRCALPHA)
            pts = []
            for j in range(10):
                r = 5.5 if j % 2 == 0 else 2.4
                a = -math.pi / 2 + j * math.pi / 5
                pts.append((6 + r * math.cos(a), 6 + r * math.sin(a)))
            pygame.draw.polygon(s, COL['yellow'], pts)
            pygame.draw.polygon(s, COL['orange'], pts, 1)
            s.set_at((5, 3), COL['white'])
            w = max(2, int(12 * abs(math.cos(ang))))
            self.stars.append(pygame.transform.scale(s, (w, 12)))
        self.ui['star_full'] = self.stars[0]
        cur = pygame.Surface((52, H), pygame.SRCALPHA)
        for y in range(H):
            w = int(50 - 26 * (y / H) ** 1.3)
            for x in range(w):
                cur.set_at((x, y), COL['red'] if (x // 5) % 2 == 0 else COL['dred'])
            cur.set_at((w, y), COL['yellow']); cur.set_at((w - 1, y), COL['orange'])
        self.ui['curtL'] = cur
        self.ui['curtR'] = pygame.transform.flip(cur, True, False)
        val = pygame.Surface((W, 28), pygame.SRCALPHA)
        for x in range(0, W + 16, 16):
            pygame.draw.circle(val, COL['yellow'], (x + 8, 15), 9)
        for x in range(0, W + 16, 16):
            pygame.draw.circle(val, COL['red'], (x + 8, 15), 8)
            pygame.draw.line(val, COL['dred'], (x + 8, 8), (x + 8, 22))
        pygame.draw.rect(val, COL['red'], (0, 0, W, 14))
        pygame.draw.rect(val, COL['yellow'], (0, 0, W, 2))
        self.ui['valance'] = val
        bg = pygame.Surface((W, H))
        for y in range(H):
            bg.fill(lerp_col((40, 16, 72), (88, 40, 120), y / 176), (0, y, W, 1))
        for yy in range(0, 176, 16):
            for xx in range((yy // 16 % 2) * 8, W, 16):
                pygame.draw.polygon(bg, (104, 56, 136), [(xx, yy + 4), (xx + 3, yy + 8), (xx, yy + 12), (xx - 3, yy + 8)])
        pygame.draw.rect(bg, (136, 84, 40), (0, 176, W, H - 176))
        for i, y in enumerate(range(176, H, 8)):
            pygame.draw.rect(bg, (152, 96, 48) if i % 2 == 0 else (128, 76, 36), (0, y, W, 8))
        for x in range(-128, W + 128, 32):
            pygame.draw.line(bg, (80, 44, 28), (W / 2 + (x - W / 2) * 0.55, 176), (x, H))
        pygame.draw.rect(bg, (80, 44, 28), (0, 174, W, 3))
        for x in range(8, W, 24):
            pygame.draw.rect(bg, COL['yellow'], (x, 176, 4, 2))
        self.ui['lobby'] = bg
        cone = pygame.Surface((90, 176), pygame.SRCALPHA)
        pygame.draw.polygon(cone, (255, 255, 200, 26), [(41, 0), (49, 0), (90, 176), (0, 176)])
        self.ui['cone'] = cone

    def build_tiles(self):
        for th in THEMES:
            t = {}
            R = pygame.draw.rect
            rnd = random.Random(5)
            g = pygame.Surface((16, 16)); g.fill(th['dirt'])
            for _ in range(5):
                R(g, th['dirt2'], (rnd.randint(1, 13), rnd.randint(7, 14), 2, 2))
            R(g, th['grass'], (0, 0, 16, 5))
            for x in range(0, 16, 4):
                R(g, th['lgrass'], (x, 0, 2, 2)); R(g, th['grass'], (x + 2, 5, 2, 2))
            R(g, th['dgrass'], (0, 4, 16, 1))
            t['G'] = g
            d = pygame.Surface((16, 16)); d.fill(th['dirt'])
            for _ in range(6):
                R(d, th['dirt2'], (rnd.randint(1, 13), rnd.randint(1, 14), 2, 2))
            t['D'] = d
            b = pygame.Surface((16, 16)); b.fill(th['brick'])
            R(b, th['brick2'], (0, 7, 16, 1)); R(b, th['brick2'], (0, 15, 16, 1))
            R(b, th['brick2'], (7, 0, 1, 7)); R(b, th['brick2'], (3, 8, 1, 7)); R(b, th['brick2'], (11, 8, 1, 7))
            R(b, lerp_col(th['brick'], COL['white'], .35), (0, 0, 16, 1))
            t['B'] = b
            p = pygame.Surface((16, 16)); p.fill(th['brick2'])
            R(p, th['lgrass'], (0, 0, 16, 4)); R(p, th['grass'], (0, 4, 16, 2))
            R(p, th['dirt'], (0, 6, 16, 6))
            for x in (2, 10):
                R(p, COL['ink'], (x, 8, 2, 2))
            R(p, th['dirt2'], (0, 12, 16, 4))
            t['='] = p
            
            qb = pygame.Surface((16, 16))
            qb.fill((248, 152, 40))
            R(qb, COL['yellow'], (1, 1, 14, 14), 1)
            draw_text(qb, '?', 4, 2, COL['white'], size='s', shadow=False)
            t['?'] = qb

            sp = pygame.Surface((16, 16), pygame.SRCALPHA)
            for x in (0, 8):
                pygame.draw.polygon(sp, (200, 200, 212), [(x, 16), (x + 4, 6), (x + 8, 16)])
                pygame.draw.polygon(sp, COL['ink'], [(x, 16), (x + 4, 6), (x + 8, 16)], 1)
                sp.set_at((x + 3, 9), COL['white'])
            t['^'] = sp
            self.tiles.append(t)

    def build_bgs(self):
        for th in THEMES:
            sky = pygame.Surface((W, H))
            sky.fill((16, 12, 32))
            for y in range(0, 176, 8):
                sky.fill(lerp_col(th['sky'][0], th['sky'][1], y / 176), (0, y, W, 8))
            rnd = random.Random(9)
            if th['night']:
                pygame.draw.circle(sky, (252, 244, 200), (200, 44), 14)
                pygame.draw.circle(sky, th['sky'][0], (194, 40), 12)
                for _ in range(50):
                    sky.set_at((rnd.randint(0, W - 1), rnd.randint(0, 120)), COL['white'])
            far = pygame.Surface((512, H), pygame.SRCALPHA)
            near = pygame.Surface((512, H), pygame.SRCALPHA)
            for x in range(512):
                a = 2 * math.pi * x / 512
                hf = 62 + 18 * math.sin(a * 2 + 1) + 9 * math.sin(a * 5)
                hn = 38 + 14 * math.sin(a * 3) + 6 * math.sin(a * 7 + 2)
                pygame.draw.line(far, th['far'], (x, GROUND_ROW * TS - int(hf)), (x, GROUND_ROW * TS))
                pygame.draw.line(near, th['near'], (x, GROUND_ROW * TS - int(hn)), (x, GROUND_ROW * TS))
            clouds = pygame.Surface((512, H), pygame.SRCALPHA)
            for _ in range(7):
                cx, cy = rnd.randint(0, 511), rnd.randint(14, 90)
                for dx, dy, r in ((0, 0, 9), (10, -3, 11), (21, 0, 9), (10, 4, 8)):
                    pygame.draw.circle(clouds, th['cloud'], (cx + dx, cy + dy), r)
                    if cx + dx + r > 512:
                        pygame.draw.circle(clouds, th['cloud'], (cx + dx - 512, cy + dy), r)
            self.bgs.append(dict(sky=sky, far=far, near=near, clouds=clouds))


def from_map(rows, pal):
    s = pygame.Surface((len(rows[0]), len(rows)), pygame.SRCALPHA)
    for y, r in enumerate(rows):
        for x, ch in enumerate(r):
            if ch in pal:
                s.set_at((x, y), pal[ch])
    return s


# ------------------------------------------------------------------- entities & powerups
class PowerUp:
    def __init__(self, x, y, kind):
        self.x, self.y, self.kind = float(x), float(y), kind
        self.vy = -2.0
        self.vx = 0.8
        self.alive = True

    def update(self, lv):
        self.vy = min(4.0, self.vy + 0.25)
        self.y += self.vy
        for tr in lv.solid_rects(self.x, self.y, 12, 12):
            if self.vy > 0:
                self.y = tr.top - 12
            self.vy = 0
        self.x += self.vx
        for tr in lv.solid_rects(self.x, self.y, 12, 12):
            if self.vx > 0:
                self.x = tr.left - 12
            elif self.vx < 0:
                self.x = tr.right
            self.vx *= -1

    def rect(self):
        return pygame.Rect(int(self.x), int(self.y), 12, 12)


class Fireball:
    def __init__(self, x, y, facing):
        self.x, self.y = float(x), float(y)
        self.vx = facing * 3.5
        self.vy = -1.0
        self.alive = True

    def update(self, lv):
        self.vy = min(4.0, self.vy + 0.3)
        self.y += self.vy
        for tr in lv.solid_rects(self.x, self.y, 6, 6):
            if self.vy > 0:
                self.y = tr.top - 6; self.vy = -2.0
            else:
                self.alive = False
        self.x += self.vx
        for tr in lv.solid_rects(self.x, self.y, 6, 6):
            self.alive = False

    def rect(self):
        return pygame.Rect(int(self.x), int(self.y), 6, 6)


class Laser:
    def __init__(self, x, y, facing):
        self.x, self.y = float(x), float(y)
        self.vx = facing * 4.0
        self.alive = True

    def update(self, lv):
        self.x += self.vx
        for tr in lv.solid_rects(self.x, self.y, 8, 4):
            self.alive = False

    def rect(self):
        return pygame.Rect(int(self.x), int(self.y), 8, 4)


class Enemy:
    def __init__(self, x, y, xmin, xmax, speed, can_shoot=False):
        self.x, self.y, self.xmin, self.xmax = float(x), y, xmin, xmax
        self.speed, self.dir, self.anim, self.squash, self.alive = speed, -1, 0, 0, True
        self.hp = 2
        self.vx = 0.0
        self.can_shoot = can_shoot
        self.shoot_cooldown = random.randint(60, 120)

    def update(self, player_x, lasers_list, audio_fx):
        if self.squash:
            self.squash -= 1
            if self.squash == 0:
                self.alive = False
            return
        if abs(self.vx) > 0.1:
            self.x += self.vx
            self.vx *= 0.85
            return

        dist = player_x - self.x
        if abs(dist) < 100:
            self.dir = 1 if dist > 0 else -1
            current_speed = self.speed * 1.5
            if self.can_shoot:
                self.shoot_cooldown -= 1
                if self.shoot_cooldown <= 0:
                    lasers_list.append(Laser(self.x, self.y + 6, self.dir))
                    audio_fx.play('laser')
                    self.shoot_cooldown = random.randint(90, 150)
        else:
            current_speed = self.speed

        self.x += self.dir * current_speed
        self.anim += 1
        if self.x <= self.xmin:
            self.x, self.dir = self.xmin, 1
        elif self.x >= self.xmax:
            self.x, self.dir = self.xmax, -1

    def rect(self):
        return pygame.Rect(int(self.x) + 2, self.y + 5, 12, 11)


class Level:
    def __init__(self, idx, art):
        self.idx, self.theme = idx, THEMES[idx]
        rng = random.Random(1000 + idx * 31)
        self.cols = [130, 160, 190][idx]
        self.rows = 14
        g = [[' '] * self.cols for _ in range(self.rows)]
        self.coins, self.enemies, self.spikes, self.powerups, self.lasers = [], [], [], [], []
        diff = idx

        def ground(a, b):
            for c in range(a, b):
                g[11][c] = 'G'; g[12][c] = 'D'; g[13][c] = 'D'

        ground(0, 14)
        for i in range(3):
            self.coins.append([(5 + i * 2) * TS + 8, 8 * TS + 8])
        x, end, last = 14, self.cols - 16, 'flat'
        while x < end:
            kind = 'flat' if last in ('gap', 'spikes') else rng.choice(
                ['flat', 'platform', 'steps', 'gap', 'spikes', 'platform', 'spikes'])
            if kind == 'flat':
                n = rng.randint(4, 8); ground(x, x + n)
                if rng.random() < 0.6:
                    for i in range(1, n - 1):
                        self.coins.append([(x + i) * TS + 8, 9 * TS + 4])
                if n >= 5 and rng.random() < 0.7 + 0.1 * diff:
                    can_shoot = (diff >= 1 and rng.random() < 0.5)
                    self.enemies.append(Enemy((x + n // 2) * TS, 11 * TS - 16, x * TS, (x + n) * TS - 16, 0.6 + 0.2 * diff, can_shoot))
                x += n; last = 'flat'
            elif kind == 'platform':
                n = rng.randint(8, 11); ground(x, x + n)
                pw = rng.randint(2, 3); px = x + rng.randint(1, n - pw - 1)
                for i in range(pw):
                    g[8][px + i] = '='
                    if rng.random() < 0.6:
                        g[7][px + i] = '?'
                if diff >= 1 or rng.random() < 0.5:
                    can_shoot = (diff >= 1 and rng.random() < 0.6)
                    self.enemies.append(Enemy((x + 1) * TS, 11 * TS - 16, x * TS, (x + n) * TS - 16, 0.65 + 0.2 * diff, can_shoot))
                x += n; last = 'flat'
            elif kind == 'steps':
                n = 10; ground(x, x + n)
                for j, h in enumerate([1, 2, 3, 4, 3, 2, 1]):
                    for k in range(h):
                        if 10 - k >= 0:
                            g[10 - k][x + 2 + j] = 'B'
                x += n; last = 'flat'
            elif kind == 'gap':
                n = rng.randint(3, 5 + diff)
                for i in range(n):
                    self.coins.append([(x + i) * TS + 8, 8 * TS + 4 + int(abs(i - (n - 1) / 2) * 8)])
                x += n; last = 'gap'
            else:
                n = 8; ground(x, x + n)
                sw = 2 + diff
                sp = x + 2
                for i in range(sw):
                    if sp + i < self.cols:
                        g[10][sp + i] = '^'
                        self.spikes.append(pygame.Rect((sp + i) * TS + 2, 10 * TS + 8, 12, 8))
                x += n; last = 'spikes'
        ground(x, self.cols)
        self.grid = g
        self.flag_x = (self.cols - 6) * TS
        self.flag_top = GROUND_ROW * TS - 80
        self.flag_rect = pygame.Rect(self.flag_x - 4, self.flag_top, 14, 80)
        self.total_stars = len(self.coins)
        tiles = art.tiles[idx]
        self.surf = pygame.Surface((self.cols * TS, H), pygame.SRCALPHA)
        for r in range(self.rows):
            for c in range(self.cols):
                ch = g[r][c]
                if ch in tiles:
                    self.surf.blit(tiles[ch], (c * TS, r * TS))

    def solid_rects(self, x, y, w, h):
        out = []
        for ty in range(math.floor(y / TS), math.floor((y + h - 0.001) / TS) + 1):
            for tx in range(math.floor(x / TS), math.floor((x + w - 0.001) / TS) + 1):
                if ty < 0 or ty >= self.rows:
                    continue
                if tx < 0 or tx >= self.cols or self.grid[ty][tx] in SOLID:
                    out.append(pygame.Rect(tx * TS, ty * TS, TS, TS))
        return out

    def draw_bg(self, c, cam, art, t):
        bg = art.bgs[self.idx]
        c.blit(bg['sky'], (0, 0))
        ox = (-int(cam * 0.1 + t * 0.08)) % 512
        c.blit(bg['clouds'], (ox - 512, 0)); c.blit(bg['clouds'], (ox, 0))
        ox = (-int(cam * 0.25)) % 512
        c.blit(bg['far'], (ox - 512, 0)); c.blit(bg['far'], (ox, 0))
        ox = (-int(cam * 0.5)) % 512
        c.blit(bg['near'], (ox - 512, 0)); c.blit(bg['near'], (ox, 0))

        if self.idx == 2 and art.custom_bg is not None:
            parallax_x = int(cam * 0.2) % W
            c.blit(art.custom_bg, (-parallax_x, 0))
            c.blit(art.custom_bg, (W - parallax_x, 0))
        else:
            # Standard procedural background for Stage 1 and Stage 2
            bg = art.bgs[self.idx]
            c.blit(bg['sky'], (0, 0))
            ox = (-int(cam * 0.1 + t * 0.08)) % 512
            c.blit(bg['clouds'], (ox - 512, 0)); c.blit(bg['clouds'], (ox, 0))
            ox = (-int(cam * 0.25)) % 512
            c.blit(bg['far'], (ox - 512, 0)); c.blit(bg['far'], (ox, 0))
            ox = (-int(cam * 0.5)) % 512
            c.blit(bg['near'], (ox - 512, 0)); c.blit(bg['near'], (ox, 0))


# ------------------------------------------------------------------ player
class Player:
    W, H = 10, 16

    def __init__(self, kind, x, y):
        self.kind, self.d = kind, HERO_DEFS[kind]
        self.x, self.y, self.vx, self.vy = float(x), float(y), 0.0, 0.0
        self.facing, self.on_ground = 1, False
        self.coyote = self.jump_buf = 0
        self.jumping = False
        self.hp, self.max_hp = self.d['hp'], self.d['hp']
        self.power = 'normal'
        self.inv, self.anim = 0, 0.0
        self.dead = False
        self.attack_timer = 0
        self.attack_type = None
        self.fireballs = []

    def rect(self):
        return pygame.Rect(int(self.x), int(self.y), self.W, self.H)

    def update(self, g):
        k, lv = g.keys, g.level
        cfg = g.controls_cfg
        left = k[cfg['left']]
        right = k[cfg['right']]
        run = k[cfg['run']]
        held = k[cfg['jump']] or k[K_UP] or k[K_w]
        
        speed_mult = 1.3 if self.power == 'speed' else 1.0
        top = (2.8 if run else 1.6) * self.d['speed'] * speed_mult
        acc = 0.12 if self.on_ground else 0.08
        friction = 0.10 if self.on_ground else 0.02

        if left and not right:
            self.vx = self.vx - acc * 1.8 if self.vx > 0 else self.vx - acc
            self.facing = -1
        elif right and not left:
            self.vx = self.vx + acc * 1.8 if self.vx < 0 else self.vx + acc
            self.facing = 1
        else:
            if self.vx > 0:
                self.vx = max(0, self.vx - friction)
            elif self.vx < 0:
                self.vx = min(0, self.vx + friction)

        self.vx = max(-top, min(top, self.vx))
        
        if (k[cfg['action']] or k[K_c] or k[K_j]) and self.attack_timer == 0:
            if self.power == 'fire':
                self.fireballs.append(Fireball(self.x, self.y, self.facing))
                g.audio.play('fireball')
                self.attack_timer = 12
                self.attack_type = 'punch'
            else:
                self.attack_timer = 14
                self.attack_type = 'punch'
                g.audio.play('punch')
                self.check_combat_hits(lv, g)
        
        if (k[cfg['special']] or k[K_k]) and not self.on_ground and self.attack_timer == 0:
            self.attack_timer = 18
            self.attack_type = 'kick'
            self.vy = -1.5
            g.audio.play('spinkick')
            self.check_combat_hits(lv, g, power=2)

        if self.attack_timer > 0:
            self.attack_timer -= 1

        for fb in self.fireballs[:]:
            fb.update(lv)
            if not fb.alive:
                self.fireballs.remove(fb)
            else:
                fbrect = fb.rect()
                for e in lv.enemies:
                    if e.alive and fbrect.colliderect(e.rect()):
                        e.squash = 16
                        fb.alive = False
                        g.audio.play('stomp')
                        g.score += 300

        self.coyote = 6 if self.on_ground else max(0, self.coyote - 1)
        if self.jump_buf > 0:
            self.jump_buf -= 1
        if self.jump_buf > 0 and self.coyote > 0:
            jump_mult = 1.25 if self.power == 'speed' else 1.0
            self.vy = -5.0 * self.d['jump'] * jump_mult
            self.jump_buf = self.coyote = 0
            self.on_ground, self.jumping = False, True
            g.audio.play('jump')

        if not held and self.vy < -1.5 and self.jumping:
            self.vy *= 0.55

        self.vy = min(5.5, self.vy + 0.28)

        self.x += self.vx
        for tr in lv.solid_rects(self.x, self.y, self.W, self.H):
            if self.vx > 0:
                self.x = tr.left - self.W
            elif self.vx < 0:
                self.x = tr.right
            self.vx = 0

        self.y += self.vy
        self.on_ground = False
        for tr in lv.solid_rects(self.x, self.y, self.W, self.H):
            if self.vy > 0:
                self.y = tr.top - self.H; self.on_ground = True; self.jumping = False
            elif self.vy < 0:
                self.y = tr.bottom
                tx, ty = tr.x // TS, (tr.y // TS)
                if lv.grid[ty][tx] == '?':
                    lv.grid[ty][tx] = 'B'
                    lv.surf.blit(g.art.tiles[lv.idx]['B'], (tx * TS, ty * TS))
                    p_kinds = ['mushroom', 'flower', 'speed', 'shield']
                    p_kind = random.choice(p_kinds)
                    lv.powerups.append(PowerUp(tx * TS, (ty - 1) * TS, p_kind))
                    g.audio.play('coin')
            self.vy = 0

        for pu in lv.powerups[:]:
            if self.rect().colliderect(pu.rect()):
                lv.powerups.remove(pu)
                g.audio.play('powerup')
                self.power = pu.kind
                if pu.kind == 'mushroom':
                    self.hp = min(self.max_hp + 2, self.hp + 2)
                elif pu.kind == 'fire':
                    self.hp = min(self.max_hp + 3, self.hp + 3)
                elif pu.kind == 'shield':
                    self.inv = 300
                g.score += 500

        for laser in lv.lasers[:]:
            laser.update(lv)
            if not laser.alive:
                lv.lasers.remove(laser)
            elif self.rect().colliderect(laser.rect()):
                lv.lasers.remove(laser)
                self.hurt(g, 1 if laser.vx > 0 else -1)

        if self.inv:
            self.inv -= 1
        self.anim += abs(self.vx) * 0.12
        if self.y > H + 30:
            g.kill_player()

    def check_combat_hits(self, lv, g, power=1):
        hit_rect = pygame.Rect(self.x + (12 if self.facing > 0 else -16), self.y, 18, self.H)
        for e in lv.enemies:
            if e.alive and hit_rect.colliderect(e.rect()):
                e.hp -= power
                e.vx = self.facing * 3.5
                g.burst(e.x + 8, e.y + 8, COL['orange'], 8)
                g.audio.play('stomp')
                if e.hp <= 0:
                    e.squash = 16
                    g.score += 300
                    g.add_popup('+300', e.x, e.y - 6)

    def hurt(self, g, kdir):
        if self.power == 'shield' or self.inv > 0 and self.power == 'shield':
            g.audio.play('hurt')
            return
        if self.inv > 0 or self.dead:
            return
        if self.power in ('fire', 'speed', 'shield'):
            self.power = 'normal'
            self.inv = 80
            g.audio.play('hurt')
            return
        elif self.power == 'mushroom':
            self.power = 'normal'
            self.inv = 80
            g.audio.play('hurt')
            return

        self.hp -= 1
        if self.hp <= 0:
            g.kill_player()
            return
        self.inv, self.vy, self.vx = 100, -3.2, kdir * 2.2
        g.audio.play('hurt')

    def pose(self):
        if self.attack_timer > 0:
            return self.attack_type
        if not self.on_ground:
            return 'jump'
        if abs(self.vx) < 0.15:
            return 'idle'
        return 'run%d' % (int(self.anim) % 3)


# -------------------------------------------------------------------- game orchestrator
class Game:
    def __init__(self):
        pygame.init()
        init_fonts()
        self.screen = pygame.display.set_mode((W * SCALE, H * SCALE))
        pygame.display.set_caption('PIKSEL PALS ADVENTURES: VOX EDITION')
        self.canvas = pygame.Surface((W, H))
        self.clock = pygame.time.Clock()
        self.audio, self.art = Audio(), Art()
        
        # State begins with introductory video cutscene
        self.state, self.t = 'intro_video', 0
        self.fade, self.fade_surf = 1.0, pygame.Surface((W, H))
        self.next_state = self.next_action = None
        self.keys = pygame.key.get_pressed()
        
        # Movie / Video setup for video_1.mp4 using OpenCV if available, else fallback timer
        self.video_cap = None
        self.video_surf = None
        try:
            import cv2
            if os.path.exists('Video project.mp4'):
                self.video_cap = cv2.VideoCapture('Video project.mp4')
        except ImportError:
            pass

        self.steps = [("PAINTING HEROES", self.art.build_heroes), ("INKING BADDIES", self.art.build_enemy),
                      ("MIXING TILE PAINT", self.art.build_tiles), ("HANGING BACKDROPS", self.art.build_bgs),
                      ("TUNING SOUND EFFECTS", self.audio.build_sfx), ("LOADING CUSTOM MP3s", self.audio.load_external_mp3s),
                      ("REHEARSING LOBBY TUNE", self.audio.build_lobby), ("REHEARSING STAGE TUNES", self.audio.build_game),
                      ("RAISING THE CURTAIN", self.art.build_ui)]
        self.load_i, self.load_disp, self.load_done = 0, 0.0, False
        self.menu_i, self.char, self.stage, self.unlocked, self.hop = 0, 0, 0, 0, 0
        self.lives, self.score, self.coins = 3, 0, 0
        self.unlocked_chars = [True, False, False]
        
        self.username = "PlayerOne"
        self.password = "••••••••"
        self.alias = "PkHero"
        self.logged_in = True
        self.controls_cfg = dict(left=K_LEFT, right=K_RIGHT, jump=K_z, run=K_LSHIFT, action=K_x, special=K_k)
        
        self.setting_i = 0
        self.binding_key = None
        self.active_input_modal = None
        self.login_field = 0
        self.auth_mode = 'login'      # 'login' or 'signup'
        self.auth_msg = ""
        self.temp_user = ""
        self.temp_pass = ""
        self.temp_alias = ""

        self.particles, self.popups, self.confetti = [], [], []
        self.level = self.player = None
        self.cam, self.banner, self.death_timer, self.clear_t = 0.0, 150, 0, 0
        self.time_left, self.got, self.bonus = 180, 0, 0
        self.tick_acc = 0

    def _apply_profile(self, data):
        self.username = data["username"]
        self.alias = data["alias"]
        self.coins = data["coins"]
        self.score = max(self.score, data["high_score"])
        self.unlocked = max(self.unlocked, data["unlocked_stage"])
        self.unlocked_chars = data["unlocked_chars"]
        self.logged_in = True

    def _auth_request(self, path, payload):
        """Returns (ok, message). The message is shown in the login box on failure."""
        try:
            res = requests.post("http://127.0.0.1:8000" + path, json=payload, timeout=3)
        except requests.RequestException:
            return False, "Server offline"
        if res.status_code == 200:
            self._apply_profile(res.json())
            return True, ""
        try:
            detail = res.json().get("detail", "Something went wrong")
        except ValueError:
            detail = "Something went wrong"
        if not isinstance(detail, str):
            detail = "Check your details"
        return False, detail

    def backend_login(self, username, password):
        return self._auth_request("/api/login", {"username": username, "password": password})

    def backend_register(self, username, password):
        return self._auth_request("/api/register", {"username": username, "password": password,
                                                    "alias": self.alias})

    def submit_auth(self):
        if not self.temp_user or not self.temp_pass:
            self.auth_msg = "Enter name and password"
            self.audio.play('no')
            return
        fn = self.backend_register if self.auth_mode == 'signup' else self.backend_login
        ok, msg = fn(self.temp_user, self.temp_pass)
        if ok:
            self.password = "•" * len(self.temp_pass)
            self.active_input_modal = None
            self.audio.play('ok')
        else:
            self.auth_msg = msg
            self.audio.play('no')

    def async_sync_progress(self):
        """Syncs player progress in a background thread so the game never freezes"""
        if not self.logged_in or self.username == "Guest":
            return
        payload = {
            "username": self.username,
            "coins": self.coins,
            "score": self.score,
            "unlocked_stage": self.unlocked,
            "unlocked_chars": self.unlocked_chars
        }
        def worker():
            try:
                requests.post("http://127.0.0.1:8000/api/sync", json=payload, timeout=3)
            except requests.RequestException:
                pass
        threading.Thread(target=worker, daemon=True).start()

    def goto(self, state, action=None):
        if self.next_state is None:
            self.next_state, self.next_action = state, action

    def start_game(self):
        self.lives = 3
        self.start_level()

    def start_level(self):
        self.level = Level(self.stage, self.art)
        self.player = Player(self.char, 2 * TS, GROUND_ROW * TS - 16)
        self.cam, self.banner, self.death_timer = 0.0, 150, 0
        self.time_left, self.got = 180, 0
        self.tick_acc = 0
        self.particles, self.popups = [], []
        
        # Play specific music per stage (Stage 2 uses mp3_9 loaded as 'stage2')
        track = THEMES[self.stage].get('music', 'game')
        self.audio.play_music(track)

    def to_lobby(self):
        self.audio.play_music('lobby')

    def kill_player(self):
        p = self.player
        if p.dead:
            return
        p.dead, p.vy, p.vx = True, -5.0, 0
        self.lives -= 1
        self.death_timer = 100
        self.audio.stop_music()
        self.audio.play('die')

    def add_popup(self, text, x, y):
        self.popups.append([text, x, y, 40])

    def burst(self, x, y, col, n=8):
        for _ in range(n):
            a = random.uniform(0, 6.28)
            sp = random.uniform(0.5, 2.0)
            self.particles.append([x, y, math.cos(a) * sp, math.sin(a) * sp - 1, random.randint(15, 30), col])

    def handle_event(self, e):
        if e.type == QUIT:
            pygame.quit(); sys.exit()
        
        if self.state == 'intro_video':
            if e.type == KEYDOWN and e.key in CONFIRM + (K_ESCAPE,):
                self.state = 'loading'
            return

        if self.state == 'settings' and self.active_input_modal is not None:
            if e.type == KEYDOWN:
                k, ch = e.key, e.unicode
                if self.active_input_modal == 'alias_modal':
                    if k == K_RETURN:
                        if self.temp_alias.strip():
                            self.alias = self.temp_alias.strip()
                        self.active_input_modal = None
                        self.audio.play('ok')
                    elif k == K_ESCAPE:
                        self.active_input_modal = None
                        self.audio.play('no')
                    elif k == K_BACKSPACE:
                        self.temp_alias = self.temp_alias[:-1]
                    elif len(self.temp_alias) < 12 and ch.isprintable() and ch != '':
                        self.temp_alias += ch
                elif self.active_input_modal == 'login_modal':
                    if k == K_ESCAPE:
                        self.active_input_modal = None
                        self.audio.play('no')
                    elif k == K_TAB or k == K_DOWN:
                        self.login_field = (self.login_field + 1) % 4
                        self.audio.play('move')
                    elif k == K_UP:
                        self.login_field = (self.login_field - 1) % 4
                        self.audio.play('move')
                    elif k == K_RETURN:
                        if self.login_field == 3:
                            self.auth_mode = 'signup' if self.auth_mode == 'login' else 'login'
                            self.auth_msg = ""
                            self.audio.play('move')
                        elif self.login_field == 2 or (self.temp_user and self.temp_pass):
                            self.submit_auth()
                        else:
                            self.login_field = (self.login_field + 1) % 4
                    elif k == K_BACKSPACE:
                        if self.login_field == 0:
                            self.temp_user = self.temp_user[:-1]
                        elif self.login_field == 1:
                            self.temp_pass = self.temp_pass[:-1]
                    elif ch.isprintable() and ch != '':
                        if self.login_field == 0 and len(self.temp_user) < 12:
                            self.temp_user += ch
                        elif self.login_field == 1 and len(self.temp_pass) < 12:
                            self.temp_pass += ch
            return

        if self.state == 'settings' and self.binding_key is not None:
            if e.type == KEYDOWN:
                k = e.key
                if k != K_ESCAPE:
                    self.controls_cfg[self.binding_key] = k
                    self.audio.play('ok')
                self.binding_key = None
            return

        if e.type != KEYDOWN:
            return
        k = e.key
        if k == K_m:
            self.audio.toggle_mute(); return
        if self.next_state:
            return
        s = self.state
        if s == 'loading':
            if self.load_done and k in CONFIRM + (K_RETURN,):
                self.audio.play('ok')
                self.goto('lobby', self.to_lobby)
        elif s == 'lobby':
            self.lobby_key(k)
        elif s == 'settings':
            self.settings_key(k)
        elif s == 'howto':
            if k in CONFIRM + (K_ESCAPE,):
                self.audio.play('ok'); self.state = 'lobby'
        elif s == 'play':
            if k in (self.controls_cfg['jump'], K_UP, K_w, K_z, K_SPACE):
                self.player.jump_buf = 7
            elif k in (K_ESCAPE, K_p):
                self.state = 'pause'
        elif s == 'pause':
            if k in (K_ESCAPE, K_p) + CONFIRM:
                self.state = 'play'
            elif k == K_q:
                self.audio.stop_music(); self.goto('lobby', self.to_lobby)
        elif s == 'clear':
            if self.clear_t > 60 and k in CONFIRM:
                if self.stage < len(THEMES) - 1:
                    self.stage += 1
                    self.goto('play', self.start_level)
                else:
                    self.goto('win', self.start_confetti)
        elif s in ('over', 'win'):
            if k in CONFIRM:
                self.goto('lobby', self.to_lobby)

    def lobby_key(self, k):
        n = 6
        if k in (K_UP, K_w):
            self.menu_i = (self.menu_i - 1) % n; self.audio.play('move')
        elif k in (K_DOWN, K_s):
            self.menu_i = (self.menu_i + 1) % n; self.audio.play('move')
        elif k in (K_LEFT, K_a, K_RIGHT, K_d):
            d = -1 if k in (K_LEFT, K_a) else 1
            self.cycle(d)
        elif k == K_u:
            self.unlocked_chars = [True, True, True]; self.audio.play('coin')
        elif k in CONFIRM:
            if self.menu_i == 0:
                if self.unlocked_chars[self.char]:
                    self.audio.play('ok'); self.audio.stop_music()
                    self.goto('play', self.start_game)
                else:
                    d = HERO_DEFS[self.char]
                    if self.coins >= d['price']:
                        self.coins -= d['price']
                        self.unlocked_chars[self.char] = True
                        self.audio.play('coin')

                        self.async_sync_progress()
                    else:
                        self.audio.play('no')
            elif self.menu_i in (1, 2):
                self.cycle(1)
            elif self.menu_i == 3:
                self.audio.play('ok'); self.state = 'settings'
            elif self.menu_i == 4:
                self.audio.play('ok'); self.state = 'howto'
            else:
                pygame.quit(); sys.exit()

    def settings_key(self, k):
        opts = ['login', 'alias', 'ctrl_left', 'ctrl_right', 'ctrl_jump', 'ctrl_run', 'ctrl_action', 'back']
        n = len(opts)
        if k in (K_UP, K_w):
            self.setting_i = (self.setting_i - 1) % n; self.audio.play('move')
        elif k in (K_DOWN, K_s):
            self.setting_i = (self.setting_i + 1) % n; self.audio.play('move')
        elif k in CONFIRM:
            sel = opts[self.setting_i]
            if sel == 'login':
                if self.logged_in:
                    self.logged_in = False
                    self.username = "Guest"
                    self.audio.play('move')
                else:
                    self.temp_user, self.temp_pass, self.login_field = "", "", 0
                    self.auth_mode, self.auth_msg = 'login', ""
                    self.active_input_modal = 'login_modal'
                    self.audio.play('ok')
            elif sel == 'alias':
                self.temp_alias = self.alias
                self.active_input_modal = 'alias_modal'
                self.audio.play('ok')
            elif sel == 'ctrl_left':
                self.binding_key = 'left'; self.audio.play('ok')
            elif sel == 'ctrl_right':
                self.binding_key = 'right'; self.audio.play('ok')
            elif sel == 'ctrl_jump':
                self.binding_key = 'jump'; self.audio.play('ok')
            elif sel == 'ctrl_run':
                self.binding_key = 'run'; self.audio.play('ok')
            elif sel == 'ctrl_action':
                self.binding_key = 'action'; self.audio.play('ok')
            elif sel == 'back':
                self.audio.play('ok'); self.state = 'lobby'

    def cycle(self, d):
        if self.menu_i == 1:
            self.char = (self.char + d) % len(HERO_DEFS); self.hop = 0; self.audio.play('move')
        elif self.menu_i == 2:
            self.stage = (self.stage + d) % (self.unlocked + 1); self.audio.play('move')

    def start_confetti(self):
        self.confetti = [[random.uniform(0, W), random.uniform(-H, 0), random.uniform(.5, 1.6),
                          random.choice([COL['yellow'], COL['pink'], COL['blue'], COL['green'], COL['orange']])]
                         for _ in range(80)]
        self.audio.play('win')

    def tick(self):
        self.t += 1
        self.keys = pygame.key.get_pressed()
        if self.next_state:
            self.fade = min(1.0, self.fade + 0.08)
            if self.fade >= 1.0:
                if self.next_action:
                    self.next_action()
                self.state, self.next_state, self.next_action = self.next_state, None, None
        elif self.fade > 0:
            self.fade = max(0.0, self.fade - 0.08)
        s = self.state
        if s == 'intro_video':
            self.update_intro_video()
        elif s == 'loading':
            self.update_loading()
        elif s == 'play':
            self.update_play()
        elif s in ('clear', 'win'):
            self.clear_t += 1
            for c in self.confetti:
                c[1] += c[2]; c[0] += math.sin(self.t * 0.05 + c[2] * 5) * 0.3
                if c[1] > H:
                    c[1] = -4
        self.draw()

    def update_intro_video(self):
        if self.video_cap is not None and self.video_cap.isOpened():
         if self.t % 2 == 0:
            ret, frame = self.video_cap.read()
            if ret:
                frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                frame = cv2.resize(frame, (W, H))
                self.video_surf = pygame.surfarray.make_surface(frame.swapaxes(0, 1))
            else:
                self.state = 'loading'
        else:
            # Fallback timer for intro video (~5 seconds)
            if self.t > 300:
                self.state = 'loading'

    def update_loading(self):
        target = self.load_i / len(self.steps)
        if self.load_i < len(self.steps) and self.load_disp >= target - 0.002:
            self.steps[self.load_i][1]()
            self.load_i += 1
            target = self.load_i / len(self.steps)
        self.load_disp = min(self.load_disp + 0.005, target)
        if self.load_i >= len(self.steps) and self.load_disp >= 0.999:
            self.load_done = True

    def update_play(self):
        lv, p = self.level, self.player
        self.banner = max(0, self.banner - 1)
        for pt in self.particles:
            pt[0] += pt[2]; pt[1] += pt[3]; pt[3] += 0.08; pt[4] -= 1
        self.particles = [q for q in self.particles if q[4] > 0]
        for po in self.popups:
            po[2] -= 0.5; po[3] -= 1
        self.popups = [q for q in self.popups if q[3] > 0]
        if p.dead:
            p.vy += 0.28; p.y += p.vy
            self.death_timer -= 1
            if self.death_timer == 0:
                if self.lives > 0:
                    self.goto('play', self.start_level)
                else:
                    self.goto('over')
            return
        p.update(self)
        for e in lv.enemies:
            e.update(p.x, lv.lasers, self.audio)
        for pu in lv.powerups:
            pu.update(lv)
        self.tick_acc += 1
        if self.tick_acc >= 45:
            self.tick_acc = 0; self.time_left -= 1
            if self.time_left <= 0:
                self.kill_player(); return
        pr = p.rect()
        for c in lv.coins[:]:
            if pr.colliderect(pygame.Rect(c[0] - 5, c[1] - 6, 10, 12)):
                lv.coins.remove(c); self.got += 1; self.score += 100; self.coins += 1
                self.audio.play('coin'); self.burst(c[0], c[1], COL['yellow'], 6)
                self.async_sync_progress()
        for sr in lv.spikes:
            if pr.colliderect(sr):
                p.hurt(self, -p.facing)
        for e in lv.enemies:
            if not e.alive or e.squash:
                continue
            if pr.colliderect(e.rect()):
                if p.vy > 0.3 and pr.bottom <= e.y + 11:
                    e.squash = 16
                    p.vy = -4.8 if (self.keys[K_z] or self.keys[K_SPACE] or self.keys[K_UP]) else -3.6
                    self.score += 200; self.audio.play('stomp')
                    self.burst(e.x + 8, e.y + 8, (200, 140, 230), 8)
                    self.add_popup('+200', e.x, e.y - 6)
                else:
                    p.hurt(self, 1 if p.x > e.x else -1)
        if pr.colliderect(lv.flag_rect):
            self.bonus = max(0, self.time_left) * 15
            self.score += self.bonus
            self.audio.play('flag')

            self.unlocked = max(self.unlocked, min(self.stage + 1, len(THEMES) - 1))

            self.async_sync_progress()

            self.state, self.clear_t = 'clear', 0
            self.audio.stop_music(); self.audio.play('win')
            self.confetti = [[random.uniform(0, W), random.uniform(-H, 0), random.uniform(.5, 1.6),
                              random.choice([COL['yellow'], COL['pink'], COL['blue'], COL['green']])] for _ in range(50)]
            return
        target = max(0, min(lv.cols * TS - W, p.x + 5 - W / 2 + p.facing * 20))
        self.cam += (target - self.cam) * 0.1

    def draw(self):
        c = self.canvas
        s = self.state
        if s == 'intro_video':
            self.draw_intro_video(c)
        elif s == 'loading':
            self.draw_loading(c)
        elif s == 'lobby':
            self.draw_lobby(c)
        elif s == 'settings':
            self.draw_settings(c)
        elif s == 'howto':
            self.draw_howto(c)
        elif s in ('play', 'pause', 'clear'):
            self.draw_play(c)
            if s == 'pause':
                self.panel(c, 'PAUSED', ['ENTER: RESUME', 'Q: BACK TO LOBBY'])
            elif s == 'clear':
                self.draw_clear(c)
        elif s == 'over':
            c.fill((12, 8, 24))
            draw_text(c, 'GAME OVER', W // 2, 80, COL['red'], 'l', 'center', sh=2)
            draw_text(c, 'FINAL SCORE %06d' % self.score, W // 2, 124, COL['white'], 'm', 'center')
            if self.t // 30 % 2:
                draw_text(c, 'PRESS ENTER', W // 2, 170, COL['yellow'], 's', 'center')
        elif s == 'win':
            self.draw_win(c)
        if self.fade > 0 and s != 'intro_video':
            self.fade_surf.set_alpha(int(self.fade * 255))
            c.blit(self.fade_surf, (0, 0))
        pygame.transform.scale(c, (W * SCALE, H * SCALE), self.screen)

    def present(self):
        pygame.display.flip()

    def draw_intro_video(self, c):
        c.fill(COL['black'])
        if self.video_surf is not None:
            c.blit(self.video_surf, (0, 0))
        else:
            # Fallback animated logo if OpenCV is missing
            draw_text(c, 'STUDIO INTRO', W // 2, H // 2 - 20, COL['white'], 'm', 'center')
            draw_text(c, 'CHECKING VIDEO...', W // 2, H // 2 + 10, COL['grey'], 's', 'center')
        draw_text(c, 'PRESS ENTER TO SKIP', W // 2, H - 20, COL['yellow'], 's', 'center')

    def draw_loading(self, c):
        c.fill((16, 16, 48))
        off = (self.t // 2) % 40
        for i in range(-80, W + 80, 40):
            pygame.draw.polygon(c, (26, 26, 74), [(i + off, 0), (i + off + 20, 0), (i + off - 40, H), (i + off - 60, H)])
        draw_text(c, 'PIKSEL PAL STUDIOS', W // 2, 18, COL['yellow'], 's', 'center')
        draw_text(c, 'PRESENTS', W // 2, 32, COL['white'], 's', 'center')
        by = int(math.sin(self.t * 0.08) * 3)
        draw_text(c, 'PIKSEL PALS', W // 2, 58 + by, COL['orange'], 'l', 'center', COL['dred'], 2)
        draw_text(c, 'THE REVENGE', W // 2, 92 - by, COL['yellow'], 'm', 'center', COL['dred'], 2)
        if self.art.stars:
            st = self.art.stars[(self.t // 5) % 8]
            for x in (36, 220):
                c.blit(st, (x - st.get_width() // 2, 76 + by))
        dots = '.' * (self.t // 15 % 4)
        draw_text(c, 'Hold on Nigga' + dots if not self.load_done else 'READY!', W // 2, 124, COL['white'], 'm', 'center')
        x0, y0, bw, bh = 28, 152, 200, 14
        pygame.draw.rect(c, COL['ink'], (x0 - 4, y0 - 4, bw + 8, bh + 8))
        for xx in range(x0 - 2, x0 + bw + 2, 8):
            pygame.draw.rect(c, (210, 210, 220), (xx, y0 - 3, 4, 2))
            pygame.draw.rect(c, (210, 210, 220), (xx, y0 + bh + 1, 4, 2))
        pygame.draw.rect(c, COL['dgrey'], (x0, y0, bw, bh))
        fw = int(bw * self.load_disp)
        if fw > 0:
            pygame.draw.rect(c, COL['orange'], (x0, y0, fw, bh))
            for xx in range(-(self.t % 16), fw, 16):
                if xx >= 0:
                    pygame.draw.rect(c, COL['yellow'], (x0 + xx, y0, min(8, fw - xx), bh))
        if self.art.heroes:
            fr = self.art.heroes[(self.t // 200) % 3][('run%d' % (self.t // 6 % 3), 1)]
            c.blit(fr, (x0 + fw - 9, y0 - 22))
        draw_text(c, '%d%%' % int(self.load_disp * 100), W // 2, 172, COL['white'], 's', 'center')
        if not self.load_done:
            lab = self.steps[min(self.load_i, len(self.steps) - 1)][0]
            draw_text(c, lab + '...', W // 2, 186, COL['grey'], 's', 'center')
        elif self.t // 25 % 2:
            draw_text(c, 'PRESS ENTER TO START', W // 2, 186, COL['yellow'], 's', 'center')
        draw_text(c, TIPS[(self.t // 200) % len(TIPS)], W // 2, 204, (168, 224, 252), 's', 'center')

    def draw_lobby(self, c):
        ui = self.art.ui
        c.blit(ui['lobby'], (0, 0))
        for i, ph in enumerate((0.0, 2.1)):
            sx = int(W / 2 - 45 + math.sin(self.t * 0.02 + ph) * 80)
            c.blit(ui['cone'], (sx, 0))
        c.blit(ui['curtL'], (0, 0)); c.blit(ui['curtR'], (W - 52, 0)); c.blit(ui['valance'], (0, 0))
        bx, by_, bw, bh = 54, 26, 148, 58
        pygame.draw.rect(c, COL['ink'], (bx, by_, bw, bh))
        pygame.draw.rect(c, (40, 24, 80), (bx + 4, by_ + 4, bw - 8, bh - 8))
        pts = [(x, by_ + 1) for x in range(bx + 3, bx + bw - 3, 8)] + [(x, by_ + bh - 4) for x in range(bx + 3, bx + bw - 3, 8)] + \
              [(bx + 1, y) for y in range(by_ + 3, by_ + bh - 3, 8)] + [(bx + bw - 4, y) for y in range(by_ + 3, by_ + bh - 3, 8)]
        for i, (x, y) in enumerate(pts):
            on = (i + self.t // 12) % 2 == 0
            pygame.draw.rect(c, COL['yellow'] if on else (120, 96, 40), (x, y, 3, 3))
        bob = int(math.sin(self.t * 0.07) * 2)
        draw_text(c, 'PIKSEL PALS', W // 2, by_ + 6 + bob, COL['orange'], 'l', 'center', COL['dred'], 2)
        draw_text(c, 'THE REVENGE', W // 2, by_ + 34, COL['yellow'], 'm', 'center', COL['dred'], 1)
        panel = pygame.Surface((124, 104), pygame.SRCALPHA); panel.fill((16, 8, 40, 190))
        c.blit(panel, (44, 92)); pygame.draw.rect(c, COL['yellow'], (44, 92, 124, 104), 1)
        d = HERO_DEFS[self.char]
        is_unlocked = self.unlocked_chars[self.char]
        status_item = 'START ADVENTURE' if is_unlocked else f'BUY: {d["price"]} COINS'
        items = [status_item, 'HERO: < %s >' % d['name'], 'STAGE: < %d >' % (self.stage + 1), 'SETTINGS', 'HOW TO PLAY', 'QUIT']
        for i, it in enumerate(items):
            sel = i == self.menu_i
            draw_text(c, it, 62, 98 + i * 14, COL['yellow'] if sel else COL['white'], 's')
            if sel:
                tx = 50 + int(math.sin(self.t * 0.2) * 2)
                pygame.draw.polygon(c, COL['yellow'], [(tx, 99 + i * 14), (tx + 6, 103 + i * 14), (tx, 107 + i * 14)])
        draw_text(c, f'COINS: {self.coins} | {self.alias}', 106, 180, COL['yellow'], 's', 'center')
        c.blit(panel, (176, 92)); pygame.draw.rect(c, COL['yellow'], (176, 92, 68, 104), 1)
        draw_text(c, d['name'] + (' (LOCKED)' if not is_unlocked else ''), 210, 96, COL['red'] if not is_unlocked else COL['white'], 's', 'center')
        self.hop += 1
        ph = self.hop % 140
        yo, pose = 0, 'idle'
        if ph < 30:
            yo, pose = -int(10 * math.sin(math.pi * ph / 30)), 'jump'
        spr = self.art.heroes[self.char][(pose, 1)]
        spr = pygame.transform.scale(spr, (36, 44))
        c.blit(spr, (192, 110 + yo))
        for i, (lab, v) in enumerate(zip(('SPD', 'JMP', 'HP'), d['bars'])):
            y = 158 + i * 12
            draw_text(c, lab, 182, y, COL['grey'], 's')
            for j in range(5):
                pygame.draw.rect(c, COL['yellow'] if j < v else (72, 56, 96), (206 + j * 7, y + 2, 5, 6))
        draw_text(c, 'ARROWS MOVE  ENTER BUY/PLAY', W // 2, 206, COL['white'], 's', 'center')

    def draw_settings(self, c):
        c.blit(self.art.ui['lobby'], (0, 0))
        c.blit(self.art.ui['curtL'], (0, 0)); c.blit(self.art.ui['curtR'], (W - 52, 0)); c.blit(self.art.ui['valance'], (0, 0))
        panel = pygame.Surface((200, 180), pygame.SRCALPHA); panel.fill((16, 8, 40, 230))
        c.blit(panel, (28, 26)); pygame.draw.rect(c, COL['yellow'], (28, 26, 200, 180), 1)
        draw_text(c, 'SETTINGS & ACCOUNT', W // 2, 32, COL['yellow'], 'm', 'center')

        login_status = f'LOGOUT ({self.username})' if self.logged_in else 'LOGIN / SIGN-UP'
        opts = [
            f'ACCOUNT: < {login_status} >',
            f'EDIT ALIAS: < {self.alias} >',
            f'CTRL LEFT: {pygame.key.name(self.controls_cfg["left"]).upper()}',
            f'CTRL RIGHT: {pygame.key.name(self.controls_cfg["right"]).upper()}',
            f'CTRL JUMP: {pygame.key.name(self.controls_cfg["jump"]).upper()}',
            f'CTRL RUN: {pygame.key.name(self.controls_cfg["run"]).upper()}',
            f'CTRL ACTION: {pygame.key.name(self.controls_cfg["action"]).upper()}',
            'BACK TO LOBBY'
        ]

        for i, opt in enumerate(opts):
            sel = (i == self.setting_i)
            col = COL['yellow'] if sel else COL['white']
            if self.binding_key and sel >= 2 and sel <= 6:
                col = COL['orange']
                opt = opt.split(':')[0] + ': [PRESS ANY KEY]'
            draw_text(c, opt, 42, 58 + i * 16, col, 's')
            if sel:
                pygame.draw.polygon(c, COL['yellow'], [(34, 59 + i * 16), (40, 63 + i * 16), (34, 67 + i * 16)])

        if self.active_input_modal == 'login_modal':
            ov = pygame.Surface((W, H), pygame.SRCALPHA); ov.fill((0, 0, 0, 180)); c.blit(ov, (0, 0))
            pygame.draw.rect(c, (24, 16, 48), (40, 40, 176, 144)); pygame.draw.rect(c, COL['yellow'], (40, 40, 176, 144), 1)
            draw_text(c, 'ACCOUNT LOGIN' if self.auth_mode == 'login' else 'CREATE ACCOUNT', W // 2, 48, COL['yellow'], 'm', 'center')
            
            draw_text(c, 'USERNAME:', 50, 75, COL['white'])
            pygame.draw.rect(c, COL['ink'], (50, 88, 156, 16))
            pygame.draw.rect(c, COL['yellow'] if self.login_field == 0 else COL['dgrey'], (50, 88, 156, 16), 1)
            draw_text(c, self.temp_user + ('_' if self.login_field == 0 and (self.t // 15 % 2) else ''), 54, 90, COL['cyan'], shadow=False)
            
            draw_text(c, 'PASSWORD:', 50, 110, COL['white'])
            pygame.draw.rect(c, COL['ink'], (50, 123, 156, 16))
            pygame.draw.rect(c, COL['yellow'] if self.login_field == 1 else COL['dgrey'], (50, 123, 156, 16), 1)
            pass_mask = '•' * len(self.temp_pass)
            draw_text(c, pass_mask + ('_' if self.login_field == 1 and (self.t // 15 % 2) else ''), 54, 125, COL['cyan'], shadow=False)
            
            if self.auth_msg:
                draw_text(c, self.auth_msg.upper()[:30], W // 2, 142, COL['red'], 's', 'center')

            sub_col = COL['yellow'] if self.login_field == 2 else COL['grey']
            sub_txt = '[ LOGIN ]' if self.auth_mode == 'login' else '[ CREATE ACCOUNT ]'
            draw_text(c, sub_txt, W // 2, 156, sub_col, 's', 'center')

            sw_col = COL['yellow'] if self.login_field == 3 else COL['grey']
            sw_txt = 'NEW HERE? CREATE ACCOUNT' if self.auth_mode == 'login' else 'HAVE AN ACCOUNT? LOGIN'
            draw_text(c, sw_txt, W // 2, 170, sw_col, 's', 'center')

        elif self.active_input_modal == 'alias_modal':
            ov = pygame.Surface((W, H), pygame.SRCALPHA); ov.fill((0, 0, 0, 180)); c.blit(ov, (0, 0))
            pygame.draw.rect(c, (24, 16, 48), (40, 65, 176, 90)); pygame.draw.rect(c, COL['yellow'], (40, 65, 176, 90), 1)
            draw_text(c, 'WRITE UNIQUE ALIAS', W // 2, 75, COL['yellow'], 'm', 'center')
            
            pygame.draw.rect(c, COL['ink'], (50, 100, 156, 18))
            pygame.draw.rect(c, COL['yellow'], (50, 100, 156, 18), 1)
            draw_text(c, self.temp_alias + ('_' if (self.t // 15 % 2) else ''), 54, 103, COL['cyan'], shadow=False)
            draw_text(c, 'PRESS ENTER TO SAVE', W // 2, 130, COL['grey'], 's', 'center')

        draw_text(c, 'ENTER TO EDIT / SELECT', W // 2, 200, COL['grey'], 's', 'center')

    def draw_howto(self, c):
        c.blit(self.art.ui['lobby'], (0, 0))
        c.blit(self.art.ui['curtL'], (0, 0)); c.blit(self.art.ui['curtR'], (W - 52, 0)); c.blit(self.art.ui['valance'], (0, 0))
        panel = pygame.Surface((176, 168), pygame.SRCALPHA); panel.fill((16, 8, 40, 220))
        c.blit(panel, (40, 34)); pygame.draw.rect(c, COL['yellow'], (40, 34, 176, 168), 1)
        draw_text(c, 'HOW TO PLAY', W // 2, 40, COL['yellow'], 'm', 'center')
        lines = ['ARROWS / WASD . MOVE', 'Z / SPACE / UP . JUMP', 'X / SHIFT ...... RUN / FIRE', 'C / K .......... PUNCH / KICK']
        for i, l in enumerate(lines):
            draw_text(c, l, 52, 64 + i * 14, COL['white'])
        c.blit(self.art.enemy[0], (52, 122)); draw_text(c, 'WATCH OUT FOR ENEMY LASERS', 76, 126)
        c.blit(self.art.tiles[0]['?'], (52, 142)); draw_text(c, 'COLLECT MUSHROOMS & SHIELDS', 76, 144)
        c.blit(self.art.tiles[0]['^'], (52, 158)); draw_text(c, 'AVOID SPIKES & PITS', 76, 162)
        draw_text(c, 'REACH THE GOAL FLAG!', W // 2, 182, COL['orange'], 's', 'center')
        if self.t // 25 % 2:
            draw_text(c, 'PRESS ENTER', W // 2, 210, COL['white'], 's', 'center')

    def draw_play(self, c):
        lv, p, cam = self.level, self.player, int(self.cam)
        lv.draw_bg(c, cam, self.art, self.t)
        c.blit(lv.surf, (0, 0), (cam, 0, W, H))
        fx = lv.flag_x - cam
        if -20 < fx < W + 20:
            pygame.draw.rect(c, COL['grey'], (fx, lv.flag_top, 3, 80))
            pygame.draw.circle(c, COL['yellow'], (fx + 1, lv.flag_top - 2), 4)
            for i in range(14):
                hgt = 14 - i // 2
                oy = int(math.sin(self.t * 0.12 + i * 0.5) * 2)
                pygame.draw.rect(c, COL['red'], (fx + 3 + i, lv.flag_top + 4 + oy, 1, hgt))
            pygame.draw.rect(c, COL['white'], (fx + 6, lv.flag_top + 8 + int(math.sin(self.t * 0.12 + 3) * 2), 2, 2))
        for s_ in lv.coins:
            sx = s_[0] - cam
            if -10 < sx < W + 10:
                fr = self.art.stars[(self.t // 6 + s_[0] // 8) % 8]
                c.blit(fr, (sx - fr.get_width() // 2, s_[1] - 6 + int(math.sin(self.t * 0.07 + s_[0]) * 1.5)))
        for pu in lv.powerups:
            pux = pu.x - cam
            if -10 < pux < W + 10:
                pucols = {'mushroom': COL['red'], 'flower': COL['fire_orange'], 'speed': COL['yellow'], 'shield': COL['cyan']}
                pygame.draw.circle(c, pucols.get(pu.kind, COL['white']), (int(pux + 6), int(pu.y + 6)), 6)
        for fb in p.fireballs:
            fbx = fb.x - cam
            if -10 < fbx < W + 10:
                pygame.draw.circle(c, COL['yellow'], (int(fbx + 3), int(fb.y + 3)), 3)
        for laser in lv.lasers:
            lx = laser.x - cam
            if -10 < lx < W + 10:
                pygame.draw.rect(c, COL['laser_red'], (int(lx), int(laser.y), 8, 4))
        for e in lv.enemies:
            if not e.alive:
                continue
            fr = self.art.enemy[(e.anim // 10) % 2]
            ex = int(e.x) - 1 - cam
            if e.squash:
                c.blit(pygame.transform.scale(fr, (18, 6)), (ex, e.y + 11))
            else:
                c.blit(fr, (ex, e.y - 1))
        if not (p.inv and (p.inv // 4) % 2 and not p.dead):
            fr = self.art.heroes[p.kind][(p.pose() if not p.dead else 'jump', p.facing)]
            if p.power == 'fire':
                fr = fr.copy()
                fr.fill(COL['fire_orange'], special_flags=pygame.BLEND_RGB_ADD)
            elif p.power == 'speed':
                fr = fr.copy()
                fr.fill(COL['yellow'], special_flags=pygame.BLEND_RGB_ADD)
            elif p.power == 'shield':
                fr = fr.copy()
                fr.fill(COL['cyan'], special_flags=pygame.BLEND_RGB_ADD)
            c.blit(fr, (int(p.x) + 5 - 9 - cam, int(p.y) + 16 - 21))
        for pt in self.particles:
            pygame.draw.rect(c, pt[5], (int(pt[0] - cam), int(pt[1]), 2, 2))
        for po in self.popups:
            draw_text(c, po[0], int(po[1] - cam), int(po[2]), COL['white'])
        self.draw_hud(c)
        if self.banner:
            off = 0 if 20 < self.banner < 130 else (min(self.banner, 150 - self.banner) - 20) * 8
            draw_text(c, 'STAGE %d' % (self.stage + 1), W // 2 + off, 52, COL['yellow'], 'l', 'center', sh=2)
            draw_text(c, THEMES[self.stage]['name'], W // 2 + off, 82, COL['white'], 'm', 'center')

    def draw_hud(self, c):
        bar = pygame.Surface((W, 20), pygame.SRCALPHA); bar.fill((0, 0, 0, 130))
        c.blit(bar, (0, 0))
        p = self.player
        for i in range(p.max_hp):
            c.blit(self.art.hearts['full' if i < p.hp else 'empty'], (3 + i * 12, 4))
        head = pygame.transform.scale(self.art.heroes[p.kind][('idle', 1)], (9, 11))
        c.blit(head, (54, 4)); draw_text(c, 'x%d' % max(0, self.lives), 66, 5)
        draw_text(c, f'{self.alias}', 94, 5, COL['yellow'])
        draw_text(c, 'T%03d' % max(0, self.time_left), 226, 5, COL['red'] if self.time_left < 30 else COL['white'])

    def panel(self, c, title, lines):
        ov = pygame.Surface((W, H), pygame.SRCALPHA); ov.fill((0, 0, 0, 140)); c.blit(ov, (0, 0))
        pygame.draw.rect(c, (16, 8, 40), (48, 70, 160, 84)); pygame.draw.rect(c, COL['yellow'], (48, 70, 160, 84), 1)
        draw_text(c, title, W // 2, 78, COL['yellow'], 'm', 'center')
        for i, l in enumerate(lines):
            draw_text(c, l, W // 2, 108 + i * 14, COL['white'], 's', 'center')

    def draw_clear(self, c):
        for q in self.confetti:
            pygame.draw.rect(c, q[3], (int(q[0]), int(q[1]), 3, 3))
        ov = pygame.Surface((W, H), pygame.SRCALPHA); ov.fill((0, 0, 0, 120)); c.blit(ov, (0, 0))
        pygame.draw.rect(c, (16, 8, 40), (36, 56, 184, 120)); pygame.draw.rect(c, COL['yellow'], (36, 56, 184, 120), 1)
        draw_text(c, 'STAGE CLEAR!', W // 2, 64, COL['yellow'], 'm', 'center')
        rows = [('COINS EARNED', '%d' % self.coins),
                ('TIME BONUS', '%d' % self.bonus), ('SCORE', '%06d' % self.score)]
        for i, (a, b) in enumerate(rows):
            if self.clear_t > 15 + i * 15:
                draw_text(c, a, 54, 94 + i * 16, COL['white']); draw_text(c, b, 202, 94 + i * 16, COL['orange'], 's', 'right')
        if self.clear_t > 60 and self.t // 25 % 2:
            last = self.stage >= len(THEMES) - 1
            draw_text(c, 'PRESS ENTER' if not last else 'ENTER: FINALE', W // 2, 150, COL['white'], 's', 'center')

    def draw_win(self, c):
        for y in range(H):
            c.fill(lerp_col((40, 16, 100), (248, 136, 120), y / H), (0, y, W, 1))
        for q in self.confetti:
            pygame.draw.rect(c, q[3], (int(q[0]), int(q[1]), 3, 3))
        bob = int(math.sin(self.t * 0.08) * 3)
        draw_text(c, 'CONGRATULATIONS!', W // 2, 50 + bob, COL['yellow'], 'l', 'center', COL['dred'], 2)
        draw_text(c, f'WELL DONE, {self.alias}!', W // 2, 92, COL['white'], 'm', 'center')
        draw_text(c, 'FINAL SCORE %06d' % self.score, W // 2, 116, COL['orange'], 'm', 'center')
        fr = self.art.heroes[self.char][('jump' if self.t // 20 % 2 else 'idle', 1)]
        c.blit(pygame.transform.scale(fr, (54, 66)), (W // 2 - 27, 140 - (6 if self.t // 20 % 2 else 0)))
        draw_text(c, 'THANKS FOR PLAYING!', W // 2, 214 - 14, COL['white'], 's', 'center')


def main():
    g = Game()
    while True:
        for e in pygame.event.get():
            g.handle_event(e)
        g.tick()
        g.present()
        g.clock.tick(FPS)


if __name__ == 'main' or __name__ == '__main__':
    main()