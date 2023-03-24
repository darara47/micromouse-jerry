#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(4);

int czujnikOdbiciowy_L, czujnikOdbiciowy_FL, czujnikOdbiciowy_FR, czujnikOdbiciowy_R, czujnikOdbiciowy_F = 0;
long enkoderLiczbaImpulsow_L, enkoderLiczbaImpulsow_R = 0;
long enkoderLiczbaImpulsowPoprzednia_L, enkoderLiczbaImpulsowPoprzednia_R = 0;
int enkoderRoznicaImpulsow_L, enkoderRoznicaImpulsow_R = 0;
#define L1 9
#define L2 8
#define R1 4
#define R2 7
#define PWM_L 6
#define PWM_R 5
int enkoderL_kierunekPrzod = 1;
int enkoderR_kierunekPrzod = 1;

byte wspolrzednaX_start = 0;
byte wspolrzednaY_start = 2;
byte zwrot_start = 1;
byte wspolrzednaX_koniec = 9;
byte wspolrzednaY_koniec = 2;
byte wymiarLabiryntu_x = 11;
byte wymiarLabiryntu_y = 11;

byte przyspieszenie = 1;
byte predkosc = 0;
byte predkoscProstoMaks = 6;
byte predkoscObrotuMaks = 4;
int enkoderOdcinkowaLiczbaImpulsow_L, enkoderOdcinkowaLiczbaImpulsow_R = 0;

bool czyPrzyspiesza = 0;
int drogaPrzyspieszania = 0;
float drogaOdcinka = 0;
bool przejechanoOdcinek = 1;
int etapOdcinka = 0;
float nastawaPoruszania = 0;
float uchybPoruszaniaPoprzedni = 0;
float uchybPoruszaniaRoznica = 0;
float wspolczynnikRegulatoraPoruszania_kp = 0.3428;
float wspolczynnikRegulatoraPoruszania_kd = -0.01334;
float wspolczynnikRegulatoraPoruszania_n = 4.31;


byte stanPrzycisku_1 = 0;
byte stanPrzycisku_2 = 0;
byte stanPrzycisku_3 = 0;
bool stanPoprzedniPrzycisku_1 = 0;
bool stanPoprzedniPrzycisku_2 = 0;
bool stanPoprzedniPrzycisku_3 = 0;


unsigned long aktualnyCzas = 0;
unsigned long zapamietanyCzas = 0;
unsigned long roznicaCzasu = 0;
unsigned long czasStartu = 0;

int dane = 0;

float wspolczynnikRegulatoraSilnika_L_kp = 0.015;
float wspolczynnikRegulatoraSilnika_L_ki = 1.5;
float uchybSilnika_L = 0;
float sumowanyUchybSilnika_L = 0;
float wymuszenieSilnika_L = 0;

float wspolczynnikRegulatoraSilnika_R_kp = 0.015;
float wspolczynnikRegulatoraSilnika_R_ki = 1.5;
float uchybSilnika_R = 0;
float sumowanyUchybSilnika_R = 0;
float wymuszenieSilnika_R = 0;

bool jestSciana_L = 0;
bool jestSciana_F = 0;
bool jestSciana_R = 0;

byte kierunekJazdy = 0;   // 0 wybierz, 1 w lewo, 2 prosto, 3 w prawo, 4 zawroc, 5 zapis
bool moznaWjechacDoZadanejKomorki = 0;
bool dojechal = 0;
bool kierunekJazdy1_sprawdzony = 0;
bool kierunekJazdy2_sprawdzony = 0;
bool kierunekJazdy3_sprawdzony = 0;

byte zwrot = zwrot_start;
byte aktualnaWspolrzednaX = wspolrzednaX_start;
byte aktualnaWspolrzednaY = wspolrzednaY_start;
byte wspolrzednaX_docelowa = wspolrzednaX_koniec;
byte wspolrzednaY_docelowa = wspolrzednaY_koniec;


byte pozycja_wskaznika = 1;
byte pop_pozycja_wskaznika = 2;
byte pozycja_wskaznika2 = 1;
byte pop_pozycja_wskaznika2 = 2;
byte trybPracyRobota = 0;   // 0 uruchomienie, 1 wyswietl zmienne na ekranie, 2 narysuj labirynt na ekranie, 3 ustawienia, 4 skanowanie, 5 rozwiazywanie, 6 czyszczenie opon, 10 koniec przejazdow
bool flaga_menu_wybranie = 0;
byte ustawienia_miganie = 0;
byte menu_miganie = 1;
bool plus_jeden = 0;
bool minus_jeden = 0;


bool tablicaScianPoziomych[11][11] = {0};
bool tablicaScianPionowych[11][11] = {0};

byte wyznaczonaTrasa[11][11] = {0};

void setup() {
  // CZUJNIKI
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  // SILNIKI
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(PWM_L, OUTPUT);
  pinMode(PWM_R, OUTPUT);

  // ENKODERY
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  attachInterrupt(0, przerwanieEnkodera_L, RISING);
  attachInterrupt(1, przerwanieEnkodera_R, RISING);

  // LED0
  pinMode(13, OUTPUT);
  // S1
  pinMode(12, INPUT);
  // S2
  pinMode(10, INPUT);
  // S3
  pinMode(11, INPUT);
  
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.clearDisplay();
  display.display();
  randomSeed(analogRead(0));

  wyznaczanieTrasy(wspolrzednaX_start, wspolrzednaY_start, wspolrzednaX_koniec, wspolrzednaY_koniec);
}

void loop() {
  
  //Serial.println((aktualnyCzas-zapamietanyCzas));
  
  dane = Serial.read();
  
  if(dane>=0) {
    Serial.print(dane);
    Serial.print("  ");
  }
  
  ustawKierunekSilnikaL_przod();
  ustawKierunekSilnikaR_przod();

  aktualnyCzas = millis();
  roznicaCzasu = aktualnyCzas - zapamietanyCzas;

  odczytPrzyciskow();

  
  if (trybPracyRobota == 4 || trybPracyRobota == 5) {
    if (stanPrzycisku_1 || stanPrzycisku_2 || stanPrzycisku_3) {
      trybPracyRobota = 0;
      odczytPrzyciskow();
    }
    if (aktualnaWspolrzednaX == wspolrzednaX_koniec && aktualnaWspolrzednaY == wspolrzednaY_koniec && !dojechal) {
      if (trybPracyRobota == 5) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(36, 16);
        display.print((millis()-czasStartu)/1000);
        display.print(".");
        display.print((millis()-czasStartu)%1000);
        display.print(" sek");
        display.display();
      }
      wspolrzednaX_docelowa = wspolrzednaX_start;
      wspolrzednaY_docelowa = wspolrzednaY_start;
      odczytScian();
      zapisScian();
      wyznaczanieTrasy(aktualnaWspolrzednaX, aktualnaWspolrzednaY, wspolrzednaX_docelowa, wspolrzednaY_docelowa);
      dojechal = 1;
      delay(200);
    }
    if (aktualnaWspolrzednaX == wspolrzednaX_start && aktualnaWspolrzednaY == wspolrzednaY_start && dojechal) {
      if (trybPracyRobota == 5) {
        trybPracyRobota = 10;
      }
      if (trybPracyRobota == 4) {
        wspolrzednaX_docelowa = wspolrzednaX_koniec;
        wspolrzednaY_docelowa = wspolrzednaY_koniec;
        odczytScian();
        zapisScian();
        wyznaczanieTrasy(aktualnaWspolrzednaX, aktualnaWspolrzednaY, wspolrzednaX_docelowa, wspolrzednaY_docelowa);
        dojechal = 0;
        delay(1000);
      }
    }
    if (!kierunekJazdy) {
      digitalWrite(13, HIGH);
      if (zwrot == 1 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY+1] == 1) kierunekJazdy=4;
      if (zwrot == 2 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX-1][aktualnaWspolrzednaY] == 1) kierunekJazdy=4;
      if (zwrot == 3 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY-1] == 1) kierunekJazdy=4;
      if (zwrot == 4 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX+1][aktualnaWspolrzednaY] == 1) kierunekJazdy=4;

      if (!kierunekJazdy1_sprawdzony) {
        if (zwrot == 1 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX-1][aktualnaWspolrzednaY] == 1 && aktualnaWspolrzednaX) kierunekJazdy=1;
        if (zwrot == 2 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY-1] == 1 && aktualnaWspolrzednaY) kierunekJazdy=1;
        if (zwrot == 3 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX+1][aktualnaWspolrzednaY] == 1 && (aktualnaWspolrzednaX < (wymiarLabiryntu_x-1))) kierunekJazdy=1;
        if (zwrot == 4 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY+1] == 1 && (aktualnaWspolrzednaY < (wymiarLabiryntu_y-1))) kierunekJazdy=1;
      }

      if (!kierunekJazdy3_sprawdzony) {
        if (zwrot == 1 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX+1][aktualnaWspolrzednaY] == 1 && (aktualnaWspolrzednaX < (wymiarLabiryntu_x-1))) kierunekJazdy=3;
        if (zwrot == 2 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY+1] == 1 && (aktualnaWspolrzednaY < (wymiarLabiryntu_y-1))) kierunekJazdy=3;
        if (zwrot == 3 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX-1][aktualnaWspolrzednaY] == 1 && aktualnaWspolrzednaX) kierunekJazdy=3;
        if (zwrot == 4 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY-1] == 1 && aktualnaWspolrzednaY) kierunekJazdy=3;
      }

      if (!kierunekJazdy2_sprawdzony) {
        if (zwrot == 1 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY-1] == 1 && aktualnaWspolrzednaY) kierunekJazdy=2;
        if (zwrot == 2 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX+1][aktualnaWspolrzednaY] == 1 && (aktualnaWspolrzednaX < (wymiarLabiryntu_x-1))) kierunekJazdy=2;
        if (zwrot == 3 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY+1] == 1 &&(aktualnaWspolrzednaY < (wymiarLabiryntu_y-1))) kierunekJazdy=2;
        if (zwrot == 4 && wyznaczonaTrasa[aktualnaWspolrzednaX][aktualnaWspolrzednaY] - wyznaczonaTrasa[aktualnaWspolrzednaX-1][aktualnaWspolrzednaY] == 1 && aktualnaWspolrzednaX) kierunekJazdy=2;
      }
      etapOdcinka = 0;
    }
    else {
      digitalWrite(13, LOW);
    }

    if (kierunekJazdy == 1) {
      if (!jestSciana_L) {
        moznaWjechacDoZadanejKomorki = 1;
        kierunekJazdy2_sprawdzony = 0;
        kierunekJazdy3_sprawdzony = 0;
      }
      else if (!moznaWjechacDoZadanejKomorki ) {
        if (trybPracyRobota == 4) {
          odczytScian();
          zapisScian();
          wyznaczanieTrasy(aktualnaWspolrzednaX, aktualnaWspolrzednaY, wspolrzednaX_docelowa, wspolrzednaY_docelowa);
        }
        kierunekJazdy1_sprawdzony = 1;
        kierunekJazdy = 0;
      }
      if (moznaWjechacDoZadanejKomorki) {
        if (etapOdcinka < 1) {
          etapOdcinka += lewo();
        }
        else if (etapOdcinka < 2) {
          etapOdcinka += prosto();
        }
        else {
          moznaWjechacDoZadanejKomorki = 0;
          kierunekJazdy = 0;
        }
      }
    }
    if (kierunekJazdy == 2) {
      if (!jestSciana_F) {
        moznaWjechacDoZadanejKomorki = 1;
        kierunekJazdy1_sprawdzony = 0;
        kierunekJazdy3_sprawdzony = 0;
      }
      else if (!moznaWjechacDoZadanejKomorki) {
        if (trybPracyRobota == 4) {
          odczytScian();
          zapisScian();
          wyznaczanieTrasy(aktualnaWspolrzednaX, aktualnaWspolrzednaY, wspolrzednaX_docelowa, wspolrzednaY_docelowa);
        }
        kierunekJazdy2_sprawdzony = 1;
        kierunekJazdy = 0;
      }
      if (moznaWjechacDoZadanejKomorki) {
        if (etapOdcinka < 1) {
          etapOdcinka += prosto();
        }
        else {
          moznaWjechacDoZadanejKomorki = 0;
          kierunekJazdy = 0;
        }
      }
    }
    if (kierunekJazdy == 3) {
      if (!jestSciana_R) {
        moznaWjechacDoZadanejKomorki = 1;
        kierunekJazdy1_sprawdzony = 0;
        kierunekJazdy2_sprawdzony = 0;
      }
      else if (!moznaWjechacDoZadanejKomorki) {
        if (trybPracyRobota == 4) {
          odczytScian();
          zapisScian();
          wyznaczanieTrasy(aktualnaWspolrzednaX, aktualnaWspolrzednaY, wspolrzednaX_docelowa, wspolrzednaY_docelowa);
        }
        kierunekJazdy3_sprawdzony = 1;
        kierunekJazdy = 0;
      }
      if (moznaWjechacDoZadanejKomorki) {
        if (etapOdcinka < 1) {
          etapOdcinka += prawo();
        }
        else if (etapOdcinka < 2) {
          etapOdcinka += prosto();
        }
        else {
          moznaWjechacDoZadanejKomorki = 0;
          kierunekJazdy = 0;
        }
      }
    }
    if (kierunekJazdy == 4) {
      moznaWjechacDoZadanejKomorki = 1;
      kierunekJazdy1_sprawdzony = 0;
      kierunekJazdy2_sprawdzony = 0;
      kierunekJazdy3_sprawdzony = 0;
      if (moznaWjechacDoZadanejKomorki) {
        if (etapOdcinka < 2) {
          etapOdcinka += lewo();
        }
        else if (etapOdcinka < 3) {
          etapOdcinka += prosto();
        }
        else {
          moznaWjechacDoZadanejKomorki = 0;
          kierunekJazdy = 0;
        }
      }
    }
  }
  else if (trybPracyRobota == 6) {
    if (stanPrzycisku_1 || stanPrzycisku_2 || stanPrzycisku_3) {
      trybPracyRobota = 0;
      odczytPrzyciskow();
    }
    regulatorSilnika_L(20);
    regulatorSilnika_R(20);
  }
  else if (trybPracyRobota == 10) {
    if (stanPrzycisku_1 || stanPrzycisku_2 || stanPrzycisku_3) {
      trybPracyRobota = 0;
      odczytPrzyciskow();
    }
    regulatorSilnika_L(0);
    regulatorSilnika_R(0);
  }
  else {
    digitalWrite(13, LOW);
    regulatorSilnika_L(0);
    regulatorSilnika_R(0);
    menu();
  }

  if (zwrot == 0) zwrot=4;
  if (zwrot == 5) zwrot=1;
  
  odczytScian();
  if ((!kierunekJazdy || kierunekJazdy == 5) && trybPracyRobota == 4) {
    if (kierunekJazdy == 5) kierunekJazdy = 0;
    zapisScian();
    //digitalWrite(13, LOW);
  }
    
  if (roznicaCzasu >= 20) {
    zapamietanyCzas = aktualnyCzas;
    enkoderRoznicaImpulsow_L = enkoderLiczbaImpulsow_L - enkoderLiczbaImpulsowPoprzednia_L;
    enkoderRoznicaImpulsow_R = enkoderLiczbaImpulsow_R - enkoderLiczbaImpulsowPoprzednia_R;
    enkoderLiczbaImpulsowPoprzednia_L = enkoderLiczbaImpulsow_L;
    enkoderLiczbaImpulsowPoprzednia_R = enkoderLiczbaImpulsow_R;
  }
}

void przerwanieEnkodera_L() {
  if (enkoderL_kierunekPrzod) {
    enkoderLiczbaImpulsow_L++;
    enkoderOdcinkowaLiczbaImpulsow_L++;
  }
  else {
    enkoderLiczbaImpulsow_L++;
    enkoderOdcinkowaLiczbaImpulsow_L++;
  }
}
void przerwanieEnkodera_R() {
if (enkoderR_kierunekPrzod) {
    enkoderLiczbaImpulsow_R++;
    enkoderOdcinkowaLiczbaImpulsow_R++;
  }
  else {
    enkoderLiczbaImpulsow_R++;
    enkoderOdcinkowaLiczbaImpulsow_R++;
  }
}
void ustawKierunekSilnikaL_przod() {
  digitalWrite(L1, HIGH);
  digitalWrite(L2, LOW);
  enkoderL_kierunekPrzod = 1;
}
void ustawKierunekSilnikaR_przod() {
  digitalWrite(R1, HIGH);
  digitalWrite(R2, LOW);
  enkoderR_kierunekPrzod = 1;
}
void ustawKierunekSilnikaL_tyl() {
  digitalWrite(L1, LOW);
  digitalWrite(L2, HIGH);
  enkoderL_kierunekPrzod = 0;
}
void ustawKierunekSilnikaR_tyl() {
  digitalWrite(R1, LOW);
  digitalWrite(R2, HIGH);
  enkoderR_kierunekPrzod = 0;
}

void wysw_zmienne() {
  display.setCursor(18,0);
  display.print("FL: ");
  display.print(czujnikOdbiciowy_FL);
  display.setCursor(70,0);
  display.print("FR: ");
  display.println(czujnikOdbiciowy_FR);
  display.print("L: ");
  display.print(czujnikOdbiciowy_L);
  display.setCursor(92,8);
  display.print("R: ");
  display.print(czujnikOdbiciowy_R);
  display.setCursor(48,8);
  display.print("B: ");
  display.print(dane);
  display.setCursor(0,16);
  display.print("EL: ");
  display.print(enkoderRoznicaImpulsow_L);
  display.setCursor(92,16);
  display.print("ER: ");
  display.print(enkoderRoznicaImpulsow_R);
  display.setCursor(44,16);
  display.print("V: ");
  display.print((aktualnyCzas-zapamietanyCzas));
  display.setCursor(0,24);
  display.print("Z: ");
  display.print(zwrot);
  display.setCursor(28,24);
  display.print("X: ");
  display.print(aktualnaWspolrzednaX);
  display.setCursor(56,24);
  display.print("Y: ");
  display.print(aktualnaWspolrzednaY);
  display.display();
}
void wysw_labirynt() {
  display.drawRect(19, 8, 91, 16, WHITE);
  for(int ix=0; ix<10; ix++) {
    for(int iy=0; iy<3; iy++) {
      if (tablicaScianPoziomych[ix][iy]) {
        for (int i=0; i<9; i++) {
          display.drawPixel((ix*9+19+i), (8+(iy*5)), WHITE);
        }
      }
      if (tablicaScianPionowych[ix][iy]) {
        for (int i=0; i<5; i++) {
          display.drawPixel((ix*9+28), (8+i+(iy*5)), WHITE);
        }
      }
    }
  }
  display.display();
}

void odczytPrzyciskow() {
  if (digitalRead(12) == LOW && stanPoprzedniPrzycisku_1) {
    stanPrzycisku_1 = 1;
  }
  else {
    stanPrzycisku_1 = 0;
  }
  stanPoprzedniPrzycisku_1 = digitalRead(12);
  
  if (digitalRead(10) == LOW && stanPoprzedniPrzycisku_2) {
    stanPrzycisku_2 = 1;
  }
  else {
    stanPrzycisku_2 = 0;
  }
  stanPoprzedniPrzycisku_2 = digitalRead(10);
  
  if (digitalRead(11) == LOW && stanPoprzedniPrzycisku_3) {
    stanPrzycisku_3 = 1;
  }
  else {
    stanPrzycisku_3 = 0;
  }
  stanPoprzedniPrzycisku_3 = digitalRead(11);
}

void odczytScian() {
  czujnikOdbiciowy_L = analogRead(A0);
  czujnikOdbiciowy_FL = analogRead(A1);
  czujnikOdbiciowy_FR = analogRead(A2);
  czujnikOdbiciowy_R = analogRead(A3);
  czujnikOdbiciowy_F = (czujnikOdbiciowy_FL + czujnikOdbiciowy_FR)/2;

  jestSciana_L = 0;
  jestSciana_F = 0;
  jestSciana_R = 0;
  
  if (czujnikOdbiciowy_L > 200) {
    jestSciana_L = 1;
  }
  if (czujnikOdbiciowy_F > 200) {
    jestSciana_F = 1;
  }
  if (czujnikOdbiciowy_R > 200) {
    jestSciana_R = 1;
  }
}

void zapisScian() {
  if (zwrot == 1) {
    if (jestSciana_F) {
      tablicaScianPoziomych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 1;
    }
    else {
      tablicaScianPoziomych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 0;
    }
    if (jestSciana_R) {
      tablicaScianPionowych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 1;
    }
    else {
      tablicaScianPionowych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 0;
    }
    if (aktualnaWspolrzednaX) {
      if (jestSciana_L) {
        tablicaScianPionowych[(aktualnaWspolrzednaX-1)][aktualnaWspolrzednaY] = 1;
      }
      else {
        tablicaScianPionowych[(aktualnaWspolrzednaX-1)][aktualnaWspolrzednaY] = 0;
      }
    }
  }
  else if (zwrot == 2) {
    if (jestSciana_L) {
      tablicaScianPoziomych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 1;
    }
    else {
      tablicaScianPoziomych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 0;
    }
    if (jestSciana_F) {
      tablicaScianPionowych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 1;
    }
    else {
      tablicaScianPionowych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 0;
    }
    if (aktualnaWspolrzednaY < (wymiarLabiryntu_y-1)) {
      if (jestSciana_R) {
        tablicaScianPoziomych[aktualnaWspolrzednaX][(aktualnaWspolrzednaY+1)] = 1;
      }
      else {
        tablicaScianPoziomych[aktualnaWspolrzednaX][(aktualnaWspolrzednaY+1)] = 0;
      }
    }
  }
  else if (zwrot == 3) {
    tablicaScianPoziomych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 0;
    if (jestSciana_L) {
      tablicaScianPionowych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 1;
    }
    else {
      tablicaScianPionowych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 0;
    }
    if (aktualnaWspolrzednaX) {
      if (jestSciana_R) {
        tablicaScianPionowych[(aktualnaWspolrzednaX-1)][aktualnaWspolrzednaY] = 1;
      }
      else {
        tablicaScianPionowych[(aktualnaWspolrzednaX-1)][aktualnaWspolrzednaY] = 0;
      }
    }
    if (aktualnaWspolrzednaY < (wymiarLabiryntu_y-1)) {
      if (jestSciana_F) {
        tablicaScianPoziomych[aktualnaWspolrzednaX][(aktualnaWspolrzednaY+1)] = 1;
      }
      else {
        tablicaScianPoziomych[aktualnaWspolrzednaX][(aktualnaWspolrzednaY+1)] = 0;
      }
    }
  }
  else if (zwrot == 4) {
    if (jestSciana_R) {
      tablicaScianPoziomych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 1;
    }
    else {
      tablicaScianPoziomych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 0;
    }
    tablicaScianPionowych[aktualnaWspolrzednaX][aktualnaWspolrzednaY] = 0;
    if (aktualnaWspolrzednaX) {
      if (jestSciana_F) {
        tablicaScianPionowych[(aktualnaWspolrzednaX-1)][aktualnaWspolrzednaY] = 1;
      }
      else {
        tablicaScianPionowych[(aktualnaWspolrzednaX-1)][aktualnaWspolrzednaY] = 0;
      }
    }
    if (aktualnaWspolrzednaY < (wymiarLabiryntu_y-1)) {
      if (jestSciana_L) {
        tablicaScianPoziomych[aktualnaWspolrzednaX][(aktualnaWspolrzednaY+1)] = 1;
      }
      else {
        tablicaScianPoziomych[aktualnaWspolrzednaX][(aktualnaWspolrzednaY+1)] = 0;
      }
    }
  }
}

void wyznaczanieTrasy(int wyznaczanieTrasyX_poczatkowe, int wspolrzednaY_poczatkowa, int wyznaczanieTrasyX_docelowe, int wyznaczanieTrasyY_docelowe) {
  for (int iy = 0; iy < wymiarLabiryntu_y; iy++) {
    for (int ix = 0; ix < wymiarLabiryntu_x; ix++) {
      wyznaczonaTrasa[ix][iy] = 0;
    }
  }
  wyznaczonaTrasa[wyznaczanieTrasyX_docelowe][wyznaczanieTrasyY_docelowe] = 1;
  for (int indeksPropagacji = 1; !wyznaczonaTrasa[wyznaczanieTrasyX_poczatkowe][wspolrzednaY_poczatkowa]; indeksPropagacji++) {
    for (int iy = 0; iy < wymiarLabiryntu_y; iy++) {
      for (int ix = 0; ix < wymiarLabiryntu_x; ix++) {
        if (wyznaczonaTrasa[ix][iy] == indeksPropagacji) {

          if (ix + 1 >= 0 && ix + 1 < wymiarLabiryntu_x && !wyznaczonaTrasa[ix + 1][iy]) {
            if (tablicaScianPionowych[ix][iy] == 0) {
              wyznaczonaTrasa[ix + 1][iy] = indeksPropagacji + 1;
            }
          }
          if (iy - 1 >= 0 && iy - 1 < wymiarLabiryntu_y && !wyznaczonaTrasa[ix][iy - 1]) {
            if (tablicaScianPoziomych[ix][iy] == 0) {
              wyznaczonaTrasa[ix][iy - 1] = indeksPropagacji + 1;
            }
          }
          if (ix - 1 >= 0 && ix - 1 < wymiarLabiryntu_x && !wyznaczonaTrasa[ix - 1][iy]) {
            if (tablicaScianPionowych[ix - 1][iy] == 0) {
              wyznaczonaTrasa[ix - 1][iy] = indeksPropagacji + 1;
            }
          }
          if (iy + 1 >= 0 && iy + 1 < wymiarLabiryntu_y && !wyznaczonaTrasa[ix][iy + 1]) {
            if (tablicaScianPoziomych[ix][iy+1] == 0) {
              wyznaczonaTrasa[ix][iy + 1] = indeksPropagacji + 1;
            }
          }
        }
      }
    }
  }
}

void menu() {
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (trybPracyRobota == 0) {
    sumowanyUchybSilnika_L = 0;
    sumowanyUchybSilnika_R = 0;
    
    display.clearDisplay();
    display.setCursor(50,0);
    display.print("MENU");
    display.drawLine(0, 8, 128, 8, WHITE);
    display.drawLine(10, 8, 10, 64, WHITE);
    
    if (stanPrzycisku_1 && pozycja_wskaznika > 1) {
      pop_pozycja_wskaznika = pozycja_wskaznika;
      pozycja_wskaznika--;
    }
    if (stanPrzycisku_3 && pozycja_wskaznika < 6) {
      pop_pozycja_wskaznika = pozycja_wskaznika;
      pozycja_wskaznika++;
    }
    if (stanPrzycisku_2) {
      trybPracyRobota = pozycja_wskaznika;
    }
  
    display.fillTriangle(
    0, 5*(pozycja_wskaznika - pop_pozycja_wskaznika + 1) + 10,
    6, 5*(pozycja_wskaznika - pop_pozycja_wskaznika + 1) + 13,
    0, 5*(pozycja_wskaznika - pop_pozycja_wskaznika + 1) + 16, WHITE);
  
    for (int i=1, y=1; i<7; i++) {
      if (pozycja_wskaznika > pop_pozycja_wskaznika) {
        if (pozycja_wskaznika < 3) {
          y = 1;
        }
        else {
          y = pozycja_wskaznika - 1;
        }
      }
      if (pozycja_wskaznika < pop_pozycja_wskaznika) {
        y = pozycja_wskaznika;
      }
      if (i==1 && y < 2) {
        display.setCursor(14, (20 - (10*y)));
        display.print("Wyswietl zmienne");
      }
      if (i==2 && y < 3) {
        display.setCursor(14, (30 - (10*y)));
        display.print("Wyswietl labirynt");
      }
      if (i==3 && y < 4) {
        display.setCursor(14, (40 - (10*y)));
        display.print("Ustawienia");
      }
      if (i==4 && y < 5) {
        display.setCursor(14, (50 - (10*y)));
        display.print("Skanuj");
      }
      if (i==5 && y < 6) {
        display.setCursor(14, (60 - (10*y)));
        display.print("Rozwiaz");
      }
      if (i==6 && y < 7) {
        display.setCursor(14, (70 - (10*y)));
        display.print("Czyszczenie opon");
      }
    }
  }
  else if (trybPracyRobota == 1) {
    display.clearDisplay();
    wysw_zmienne();
    if (stanPrzycisku_1 || stanPrzycisku_2 || stanPrzycisku_3) {
      trybPracyRobota = 0;
    }
  }
  else if (trybPracyRobota == 2) {
    display.clearDisplay();
    wysw_labirynt();
    if (stanPrzycisku_1 || stanPrzycisku_2 || stanPrzycisku_3) {
      trybPracyRobota = 0;
    }
  }
  else if (trybPracyRobota == 3) {
    display.clearDisplay();
    display.setCursor(34,0);
    display.print("USTAWIENIA");
    display.drawLine(0, 8, 128, 8, WHITE);
    display.drawLine(10, 8, 10, 64, WHITE);

    if (flaga_menu_wybranie) menu_miganie++;
    if (menu_miganie > 2) menu_miganie = 0;
        
    if (stanPrzycisku_1 && pozycja_wskaznika2 > 1) {
      if (!flaga_menu_wybranie) {
        pop_pozycja_wskaznika2 = pozycja_wskaznika2;
        pozycja_wskaznika2--;
      }
      else {
        plus_jeden = 1;
      }
    }
    if (stanPrzycisku_3 && pozycja_wskaznika2 < 10) {
      if (!flaga_menu_wybranie) {
        pop_pozycja_wskaznika2 = pozycja_wskaznika2;
        pozycja_wskaznika2++;
      }
      else {
        minus_jeden = 1;
      }
    }
    if (stanPrzycisku_2) {
      if (pozycja_wskaznika2 == 10) {
        trybPracyRobota = 0;
        pozycja_wskaznika2 = 1;
        pop_pozycja_wskaznika2 = 2;
      }
      else if (pozycja_wskaznika2 >= 3) {
        if (!flaga_menu_wybranie) {
          flaga_menu_wybranie = 1;
          menu_miganie = 0;
          ustawienia_miganie = pozycja_wskaznika2;
        }
        else {
          flaga_menu_wybranie = 0;
          ustawienia_miganie = 0;
        }
      }
    }
  
    display.fillTriangle(
    0, 5*(pozycja_wskaznika2 - pop_pozycja_wskaznika2 + 1) + 10,
    6, 5*(pozycja_wskaznika2 - pop_pozycja_wskaznika2 + 1) + 13,
    0, 5*(pozycja_wskaznika2 - pop_pozycja_wskaznika2 + 1) + 16, WHITE);
  
    for (int i=1, y=1; i<11; i++) {
      if (pozycja_wskaznika2 > pop_pozycja_wskaznika2) {
        if (pozycja_wskaznika2 < 3) {
          y = 1;
        }
        else {
          y = pozycja_wskaznika2 - 1;
        }
      }
      if (pozycja_wskaznika2 < pop_pozycja_wskaznika2) {
        y = pozycja_wskaznika2;
      }
      if (i==1 && y < 2) {
        display.setCursor(14, (20 - (10*y)));
        display.print("Wymiar x");
        display.setCursor(112, (20 - (10*y)));
        display.print(wymiarLabiryntu_x);
      }
      if (i==2 && y < 3) {
        display.setCursor(14, (30 - (10*y)));
        display.print("Wymiar y");
        display.setCursor(112, (30 - (10*y)));
        display.print(wymiarLabiryntu_y);
      }
      if (i==3 && y < 4) {
        display.setCursor(14, (40 - (10*y)));
        display.print("X poczatkowe");
        display.setCursor(90, (40 - (10*y)));
        if (menu_miganie || ustawienia_miganie != i) display.print(wspolrzednaX_start);
        else {
          if (plus_jeden) wspolrzednaX_start+=1, plus_jeden = 0;
          else if (minus_jeden) wspolrzednaX_start-=1, minus_jeden = 0;
        }
      }
      if (i==4 && y < 5) {
        display.setCursor(14, (50 - (10*y)));
        display.print("Y poczatkowe");
        display.setCursor(90, (50 - (10*y)));
        if (menu_miganie || ustawienia_miganie != i) display.print(wspolrzednaY_start);
        else {
          if (plus_jeden) wspolrzednaY_start+=1, plus_jeden = 0;
          else if (minus_jeden) wspolrzednaY_start-=1, minus_jeden = 0;
        }
      }
      if (i==5 && y < 6) {
        display.setCursor(14, (60 - (10*y)));
        display.print("Zwrot poczatk");
        display.setCursor(112, (60 - (10*y)));
        if (menu_miganie || ustawienia_miganie != i) display.print(zwrot_start);
        else {
          if (plus_jeden) zwrot_start++, plus_jeden = 0;
          else if (minus_jeden) zwrot_start--, minus_jeden = 0;
        }
      }
      if (i==6 && y < 7) {
        display.setCursor(14, (70 - (10*y)));
        display.print("X koncowe");
        display.setCursor(112, (70 - (10*y)));
        if (menu_miganie || ustawienia_miganie != i) display.print(wspolrzednaX_koniec);
        else {
          if (plus_jeden) wspolrzednaX_koniec++, plus_jeden = 0;
          else if (minus_jeden) wspolrzednaX_koniec--, minus_jeden = 0;
        }
      }
      if (i==7 && y < 8) {
        display.setCursor(14, (80 - (10*y)));
        display.print("Y koncowe");
        display.setCursor(112, (80 - (10*y)));
        if (menu_miganie || ustawienia_miganie != i) display.print(wspolrzednaY_koniec);
        else {
          if (plus_jeden) wspolrzednaY_koniec++, plus_jeden = 0;
          else if (minus_jeden) wspolrzednaY_koniec--, minus_jeden = 0;
        }
      }
      if (i==8 && y < 9) {
        display.setCursor(14, (90 - (10*y)));
        display.print("Vmax - prosta");
        display.setCursor(112, (90 - (10*y)));
        if (menu_miganie || ustawienia_miganie != i) display.print(predkoscProstoMaks);
        else {
          if (plus_jeden) predkoscProstoMaks++, plus_jeden = 0;
          else if (minus_jeden) predkoscProstoMaks--, minus_jeden = 0;
        }
      }
      if (i==9 && y < 10) {
        display.setCursor(14, (100 - (10*y)));
        display.print("Vmax - zakret");
        display.setCursor(112, (100 - (10*y)));
        if (menu_miganie || ustawienia_miganie != i) display.print(predkoscObrotuMaks);
        else {
          if (plus_jeden) predkoscObrotuMaks++, plus_jeden = 0;
          else if (minus_jeden) predkoscObrotuMaks--, minus_jeden = 0;
        }
      }
      if (i==10 && y < 11) {
        display.setCursor(14, (110 - (10*y)));
        display.print("Powrot");
      }
    }
  }
  if (trybPracyRobota == 4) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Skanowanie");
    resetRegulatorowSilnikow();
    resetRegulatoraPoruszania();
    kierunekJazdy1_sprawdzony = 0;
    kierunekJazdy2_sprawdzony = 0;
    kierunekJazdy3_sprawdzony = 0;
    kierunekJazdy = 5;
    moznaWjechacDoZadanejKomorki = 0;
    dojechal = 0;
    aktualnaWspolrzednaX = wspolrzednaX_start;
    aktualnaWspolrzednaY = wspolrzednaY_start;
    zwrot = zwrot_start;
    wyznaczanieTrasy(aktualnaWspolrzednaX, aktualnaWspolrzednaY, wspolrzednaX_koniec, wspolrzednaY_koniec);
  }
  if (trybPracyRobota == 5) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Przejazd");
    resetRegulatorowSilnikow();
    resetRegulatoraPoruszania();
    kierunekJazdy1_sprawdzony = 0;
    kierunekJazdy2_sprawdzony = 0;
    kierunekJazdy3_sprawdzony = 0;
    kierunekJazdy = 0;
    moznaWjechacDoZadanejKomorki = 0;
    dojechal = 0;
    aktualnaWspolrzednaX = wspolrzednaX_start;
    aktualnaWspolrzednaY = wspolrzednaY_start;
    zwrot = zwrot_start;
    wyznaczanieTrasy(aktualnaWspolrzednaX, aktualnaWspolrzednaY, wspolrzednaX_koniec, wspolrzednaY_koniec);
    czasStartu = millis();
  }
  if (trybPracyRobota == 6) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Czyszczenie");
    resetRegulatorowSilnikow();
    resetRegulatoraPoruszania();
  }
  
  display.display();
}

void regulatorSilnika_L(float zadanaIloscImpulsow) {
  if (zadanaIloscImpulsow > 0 ) {
    uchybSilnika_L = zadanaIloscImpulsow - enkoderRoznicaImpulsow_L;
    if (roznicaCzasu >= 20) {
      sumowanyUchybSilnika_L += uchybSilnika_L;
    }
    wymuszenieSilnika_L = wspolczynnikRegulatoraSilnika_L_kp * uchybSilnika_L + wspolczynnikRegulatoraSilnika_L_ki * sumowanyUchybSilnika_L;
    if (wymuszenieSilnika_L >= 0) {
      analogWrite(PWM_L, wymuszenieSilnika_L);
    }
    else {
      analogWrite(PWM_L, 0);
    }
  }
  else {
    analogWrite(PWM_L, 0);
  }
}
void regulatorSilnika_R(float zadanaIloscImpulsow) {
  if (zadanaIloscImpulsow > 0) {
    uchybSilnika_R = zadanaIloscImpulsow - enkoderRoznicaImpulsow_R;
    if (roznicaCzasu >= 20) {
      sumowanyUchybSilnika_R += uchybSilnika_R;
    }
    wymuszenieSilnika_R = wspolczynnikRegulatoraSilnika_R_kp * uchybSilnika_R + wspolczynnikRegulatoraSilnika_R_ki * sumowanyUchybSilnika_R;
    if (wymuszenieSilnika_R >= 0) {
      analogWrite(PWM_R, wymuszenieSilnika_R);
    }
    else {
      analogWrite(PWM_R, 0);
    }
  }
  else {
    analogWrite(PWM_R, 0);
  }
}

bool prosto() {
  ustawKierunekSilnikaL_przod();
  ustawKierunekSilnikaR_przod();
  if (!predkosc) {
    czyPrzyspiesza = 0;
    enkoderOdcinkowaLiczbaImpulsow_L = 0;
    enkoderOdcinkowaLiczbaImpulsow_R = 0;
    drogaPrzyspieszania = 0;
    przejechanoOdcinek = 0;
    drogaOdcinka = 214;
    //drogaOdcinka = 18*90/(2.5*PI);
  }
  if (!czyPrzyspiesza) {
    if (predkosc < predkoscProstoMaks) {
      predkosc += przyspieszenie;
    }
    if (predkosc >= predkoscProstoMaks) {
      predkosc = predkoscProstoMaks;
      drogaPrzyspieszania = ((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2);
      czyPrzyspiesza = 1;
    }
  }
  if (((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2) >= (drogaOdcinka - (1.5*drogaPrzyspieszania))) {
    if (predkosc > 2) {
      predkosc -= przyspieszenie;
    }
    if (predkosc <= 2) {
      predkosc = 2;
    }
  }
  odczytScian();
  if ((((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2) >= (drogaOdcinka)) || (czujnikOdbiciowy_F > 550)) {
    predkosc = 0;
    przejechanoOdcinek = 1;
  }
  if ((jestSciana_L) && (jestSciana_R)) {
    regulatorPoruszania((czujnikOdbiciowy_L - czujnikOdbiciowy_R) / 200.00);
  }
  else if ((!jestSciana_L) && (!jestSciana_R)) {
    nastawaPoruszania = 0;
  }
  else if (!jestSciana_L) {
    regulatorPoruszania((442 - czujnikOdbiciowy_R) / 150.00);
  }
  else if (!jestSciana_R) {
    regulatorPoruszania((czujnikOdbiciowy_L - 442) / 150.00);
  }
  regulatorSilnika_L(predkosc+nastawaPoruszania);
  regulatorSilnika_R(predkosc-nastawaPoruszania);
  if (przejechanoOdcinek) {
    regulatorSilnika_L(0);
    regulatorSilnika_R(0);
    if (zwrot == 1) aktualnaWspolrzednaY--;
    if (zwrot == 2) aktualnaWspolrzednaX++;
    if (zwrot == 3) aktualnaWspolrzednaY++;
    if (zwrot == 4) aktualnaWspolrzednaX--;
    return 1;
  }
  else {
    return 0;
  }
}

void regulatorPoruszania(float uchybPoruszania) {
  if (roznicaCzasu >= 20) {
    uchybPoruszaniaRoznica = uchybPoruszania - uchybPoruszaniaPoprzedni;
    uchybPoruszaniaPoprzedni = uchybPoruszania;
  }
  nastawaPoruszania = wspolczynnikRegulatoraPoruszania_kp * uchybPoruszania + wspolczynnikRegulatoraPoruszania_kd * wspolczynnikRegulatoraPoruszania_n / (1 + (wspolczynnikRegulatoraPoruszania_n * uchybPoruszaniaRoznica));
}

bool lewo() {
  ustawKierunekSilnikaL_tyl();
  ustawKierunekSilnikaR_przod();
  if (!predkosc) {
    czyPrzyspiesza = 0;
    enkoderOdcinkowaLiczbaImpulsow_L = 0;
    enkoderOdcinkowaLiczbaImpulsow_R = 0;
    drogaPrzyspieszania = 0;
    przejechanoOdcinek = 0;
    drogaOdcinka = 84;
  }
  if (!czyPrzyspiesza) {
    if (predkosc < predkoscObrotuMaks) {
      predkosc += przyspieszenie;
    }
    if (predkosc >= predkoscObrotuMaks) {
      predkosc = predkoscObrotuMaks;
      drogaPrzyspieszania = ((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2);
      czyPrzyspiesza = 1;
    }
  }
  if (((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2) >= (drogaOdcinka - (1.5*drogaPrzyspieszania))) {
    if (predkosc > 1) {
      predkosc -= przyspieszenie;
    }
    if (predkosc <= 1) {
      predkosc = 1;
    }
    if (((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2) >= (drogaOdcinka)) {
      predkosc = 0;
      przejechanoOdcinek = 1;
      zwrot--;
    }
  }
  regulatorSilnika_L(predkosc);
  regulatorSilnika_R(predkosc);
  if (przejechanoOdcinek) {
    regulatorSilnika_L(0);
    regulatorSilnika_R(0);
    resetRegulatoraPoruszania();
    return 1;
  }
  else {
    return 0;
  }
}

bool prawo() {
  ustawKierunekSilnikaL_przod();
  ustawKierunekSilnikaR_tyl();
  if (!predkosc) {
    czyPrzyspiesza = 0;
    enkoderOdcinkowaLiczbaImpulsow_L = 0;
    enkoderOdcinkowaLiczbaImpulsow_R = 0;
    drogaPrzyspieszania = 0;
    przejechanoOdcinek = 0;
    drogaOdcinka = 85;
  }
  if (!czyPrzyspiesza) {
    if (predkosc < predkoscObrotuMaks) {
      predkosc += przyspieszenie;
    }
    if (predkosc >= predkoscObrotuMaks) {
      predkosc = predkoscObrotuMaks;
      drogaPrzyspieszania = ((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2);
      czyPrzyspiesza = 1;
    }
  }
  if (((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2) >= (drogaOdcinka - (1.5*drogaPrzyspieszania))) {
    if (predkosc > 1) {
      predkosc -= przyspieszenie;
    }
    if (predkosc <= 1) {
      predkosc = 1;
    }
    if (((enkoderOdcinkowaLiczbaImpulsow_L + enkoderOdcinkowaLiczbaImpulsow_R) / 2) >= (drogaOdcinka)) {
      predkosc = 0;
      przejechanoOdcinek = 1;
      zwrot++;
    }
  }
  regulatorSilnika_L(predkosc);
  regulatorSilnika_R(predkosc);
  if (przejechanoOdcinek) {
    regulatorSilnika_L(0);
    regulatorSilnika_R(0);
    resetRegulatoraPoruszania();
    return 1;
  }
  else {
    return 0;
  }
}

void resetRegulatorowSilnikow() {
  sumowanyUchybSilnika_L = 0;
  sumowanyUchybSilnika_R = 0;
}
void resetRegulatoraPoruszania() {
  uchybPoruszaniaRoznica = 0;
}
