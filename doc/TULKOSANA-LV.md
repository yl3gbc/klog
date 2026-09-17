# KLog tulkošana latviski

Instrukcija YL3AUG. Ubuntu 26.04, Cinnamon darbvirsma.

## 1. Sagatavošana (jādara vienreiz)

Termināli atver ar **Ctrl+Alt+T**.

### 1.1. Git iestatījumi

    git config --global user.name "Vards Uzvards"
    git config --global user.email "tavs@epasts.lv"
    git config --global credential.helper store

Pēdējā rinda nozīmē, ka paroli prasīs tikai pirmajā reizē.

### 1.2. GitHub tokens

Parasto paroli GitHub vairs nepieņem — vajag tokenu.

1. Atver https://github.com/settings/tokens
2. Spied **Generate new token** -> **Generate new token (classic)**
3. Note: "klog tulkosana"
4. Expiration: **No expiration**
5. Atzīmē rūtiņu **repo**
6. Lapas apakšā **Generate token**
7. **Nokopē to uzreiz** — pēc lapas aizvēršanas neredzēsi

## 2. Projekta noklonēšana (arī vienreiz)

    cd ~
    git clone https://github.com/yl3gbc/klog.git klog-lv
    cd klog-lv
    git checkout klog-lv-work-3

Tulkojamais fails:

    ~/klog-lv/src/translations/klog_lv.ts

## 3. Katru reizi, pirms sāc tulkot

    cd ~/klog-lv
    git pull

**Svarīgi.** Ja to neizdarīsi, tavs darbs var pārrakstīt to, ko pa to
laiku iztulkojis kāds cits.

## 4. Tulkošana

Atver **Qt Linguist** un tajā failu:

    ~/klog-lv/src/translations/klog_lv.ts

Pēc katras iztulkotās virknes nospied **zaļo ķeksi** (Ctrl+Enter).
Bez tā virkne skaitās nepabeigta, un programma rādīs angliski.
Saglabā ar **Ctrl+S**.

### Ko ievērot

- `%1`, `%2` ir vietturi — atstāj tos
- `&` pirms burta ir īsinājumtaustiņš
- `\n` ir rindas pārtraukums — atstāj
- Ja nezini terminu, atstāj neiztulkotu un pasaki YL3GBC

### Termini

    band          diapazons
    callsign      izsaukuma signāls
    locator, grid lokators
    mode          režīms
    log           žurnāls
    award         diploms
    worked        strādāts
    confirmed     apstiprināts
    queued        rindā
    upload        augšupielāde
    download      lejupielāde

## 5. Kad esi pabeidzis

    cd ~/klog-lv
    git add src/translations/klog_lv.ts
    git commit -m "Latviesu tulkojums"
    git push

Pirmajā push reizē prasīs:

    Username: tavs GitHub lietotajvards
    Password: <ielīmē TOKENU, ne paroli>

Turpmāk vairs neprasīs.

### Ja push neizdodas

    git pull --rebase
    git push

## 6. Biežākās kļūdas

**Aizmirsts zaļais ķeksis** — virkne iztulkota, bet programma rāda
angliski.

**Aizmirsts git pull** — tavs darbs pārraksta citu darbu.

**Linguist atvērts, kamēr dari git pull** — aizver to pirms pull.

## Jautājumi

YL3GBC, Arnis.
