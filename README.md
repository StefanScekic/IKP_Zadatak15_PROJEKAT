# IKP_Zadatak15_PROJEKAT

TODO:
- [X] Struktura Procesa i Replikatora 
- [X] Konekcija replikatora i procesa preko soketa
  - [X] Odabir porta za otvaranje soketa 
  - [X] Razmena podataka 
  - [X] Rad kao asinhroni Thread
- [ ] Slanje zahteva Replikatoru
- [ ] Proces : Primanje podataka od replikatora
- [ ] Podrska za obradu zahteva 
- [ ] Registracija procesa
  - [ ] Izbacivanje procesa iz liste pri gasenju konekcije 
  - [ ] Struktura za skladistenje registrovanih procesa
  - [ ] Provera postojanja procesa
- [ ] Konekcija izmedju 2 Replikatora
- [ ] Dinamicki niz za replikaciju

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