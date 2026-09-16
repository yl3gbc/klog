# Kā tulkot KLogNG latviski

## Vienreiz: sagatavošana

Uzinstalē Qt Linguist un git:

    sudo apt install qttools5-dev-tools git

Noklonē repozitoriju:

    cd ~
    git clone https://github.com/yl3gbc/klog.git klogng
    cd klogng
    git checkout klogng-main

## Katru reizi: pirms sāc tulkot

Paņem jaunāko versiju:

    cd ~/klogng
    git pull

Tas ir svarīgi — citādi tavs darbs var pārrakstīt to, kas jau iztulkots.

## Tulkošana

Atver Qt Linguist un tajā failu:

    ~/klogng/src/translations/klog_lv.ts

Katrai virknei ieraksti tulkojumu un nospied **zaļo ķeksi** (vai Ctrl+Enter).
Bez ķekša virkne skaitās nepabeigta, un programma to nerādīs.

Saglabā ar Ctrl+S.

## Kad pabeidzi

    cd ~/klogng
    git add src/translations/klog_lv.ts
    git commit -m "Latviesu tulkojums: <cik virknes vai kura sadala>"
    git push

Ja push prasa paroli, vajag GitHub tokenu — pajautā YL3GBC.
