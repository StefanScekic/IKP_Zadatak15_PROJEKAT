# IKP_Zadatak15_PROJEKAT

TODO:
- [X] Struktura Procesa i Replikatora 
- [X] Konekcija replikatora i procesa preko soketa
  - [X] Odabir porta za otvaranje soketa 
  - [X] Razmena podataka 
  - [X] Rad kao asinhroni Thread
- [X] Slanje zahteva Replikatoru
- [X] Proces : Primanje podataka od replikatora
- [X] Registracija procesa
  - [X] Izbacivanje procesa iz liste pri gasenju konekcije 
  - [X] Struktura za skladistenje registrovanih procesa
  - [X] Provera postojanja procesa
- [X] Konekcija izmedju 2 Replikatora
- [X] Obrada zahteva
- [ ] Sinhrona/Asinhrona replikacija
- [X] Dodati close_sockets() koji ce da prodje korz sve current sokcete i zatvorice ih
- [ ] Pocistiti finalnu verziju koda i dodati detaljnije opise funkcija
- [ ] Dokumentacija
- [X] Testovi

  Kao ChangeLog:
  + V1.0
    - Napravljen serverski soket koji radi u sopstvenoj niti i asinhrono obradjuje zahteve
    - Gracefull shutdown u ovoj verziji postignut
  + V1.0.1
    - MASIVAN CodeClean-up
  + V1.0.2
    - Odabir porta implementiran
  + V1.1
    - Implementiran request socket za klijenta i slanje poruka
  + V1.2
    - Implementirana hash tabela za skaldistenje procesa
    - Kreirana podrska za obradu zahteva na strani replikatora
  + V1.3
    - Print Hash Table
    - Insert u hash table update
    - Server sad podrzava sesiju klijenta
  + V1.4
    - Dodat close_sockets() koji zatvara sve konekcije pri gasenju Replikatora
    - Dodato brisanje HashTable-a pri gasenju
    - Socket za primanje podataka od replikatora 
  + V1.5
    - Odabir connection parametara za process
    - CLEANUP
  + V1.6
    - Replikacija implementirana
    - 2 Testa implementirana