# IKP_Zadatak15_PROJEKAT

TODO:
- [X] Struktura Procesa i Replikatora 
- [X] Konekcija replikatora i procesa preko soketa
  - [ ] Odabir porta za otvaranje soketa 
  - [X] Razmena podataka 
  - [X] Rad kao asinhroni Thread
- [ ] Registracija procesa
  - [ ] Izbacivanje procesa iz liste pri gasenju konekcije 

  Kao ChangeLog:
  V1.0
  - Napravljen serverski soket koji radi u sopstvenoj niti i asinhrono obradjuje zahteve
  - Gracefull shutdown u ovoj verziji postignut
  V1.1
  - MASIVAN CodeClean-up