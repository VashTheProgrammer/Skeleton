# Scheduler Framework per RP2040

## Panoramica
Questo repository contiene un framework modulare per la gestione dei task progettato per il microcontrollore RP2040. Il progetto include componenti fondamentali per la gestione dei task, algoritmi di scheduling e meccanismi di inizializzazione del sistema. Lo scheduler supporta molteplici algoritmi, offrendo flessibilità e adattabilità per una vasta gamma di applicazioni, con un approccio didattico mirato.

## Caratteristiche
- **Algoritmi di Scheduling Multipli**:
  - Basato su priorità (PRIORITY)
  - Round-robin
  - Earliest-deadline-first
  - Least-executed
  - Longest-waiting
- **Normalizzazione delle Priorità Dinamiche** per prevenire starvation dei task.
- **Metriche Dettagliate sui Task**:
  - Tempo di esecuzione
  - Jitter
  - Utilizzo della memoria
- **Interfaccia Terminale VT100** per l'interazione con l'utente.
- **Design Modulare** per facilitare l'estensione e la personalizzazione.
- **Concetti di Classe e Oggetti** adattati al linguaggio C tramite strutture e puntatori a funzione.

## Struttura del Progetto
### Organizzazione dei File
- **`main.c`**: Punto di ingresso del programma. Gestisce l'inizializzazione dell'hardware, della configurazione e dello scheduler.
- **`scheduler.h` / `scheduler_core.c`**: Logica principale dello scheduler, gestione dei task e implementazione degli algoritmi.
- **`terminal.h` / `terminal.c`**: Interfaccia utente tramite terminale VT100.
- **`initcalls.h` / `initcalls.c`**: Meccanismo modulare di inizializzazione per i componenti del sistema.
- **`config.h` / `config.c`**: Gestione dei parametri di configurazione e memorizzazione.
- **`hardware_cfg.h` / `hardware.c`**: Layer di astrazione hardware (HAL) per configurazioni specifiche della scheda.
- **`debug.h` / `debug.c`**: Strumenti di debug per il logging specifico dei task.
- **`flash.h` / `flash.c`**: Utilità per la gestione della memoria flash e la persistenza dei parametri.

### Concetti di Design Modulare
Il progetto utilizza principi di design modulare per:
- Migliorare la manutenibilità e la riusabilità del codice.
- Permettere il testing indipendente dei componenti.
- Supportare configurazioni hardware multiple.

#### Funzioni `static`
Le funzioni contrassegnate come `static` sono utilizzate esclusivamente all'interno del file in cui sono definite. Questa scelta:
- Migliora l'incapsulamento.
- Previene accessi non intenzionali da altre parti del codice.
- Riduce le dipendenze e aumenta la modularità.

#### HAL (Hardware Abstraction Layer)
Il file `hardware_cfg.h` definisce configurazioni hardware astratte, consentendo:
- La separazione tra logica applicativa e configurazione hardware.
- La facilità di passaggio tra versioni hardware diverse (es. `hardware_v1`, `hardware_v2`).

#### Approccio Simile a Classi e Oggetti
Il progetto utilizza strutture e puntatori a funzione per emulare il comportamento di classi e oggetti in linguaggi orientati agli oggetti:
- Le strutture (es. `DriverLed`) contengono sia i dati che i puntatori alle funzioni operative.
- Questo approccio favorisce la modularità e la leggibilità del codice, pur sfruttando la semplicità del linguaggio C.

### Interazioni tra i File
- **Scheduler e Task**:
  - `scheduler_core.c` gestisce l'esecuzione dei task, mentre `scheduler.h` espone l'API per aggiungere e gestire i task.
  - La logica specifica dei task risiede in moduli dedicati come `task_led.c` o `task_terminal.c`.
- **Terminale e Comandi**:
  - Il sistema terminale consente la configurazione e il monitoraggio runtime tramite comandi.
  - I comandi sono registrati in `cmd.c` e gestiti dinamicamente in base all'input dell'utente.
- **Inizializzazione**:
  - Il meccanismo di `initcalls` assicura un'ordinata inizializzazione dei componenti.

## Funzionamento dello Scheduler
1. **Aggiunta dei Task**: I task vengono aggiunti allo scheduler con nome, funzione, priorità e intervallo.
2. **Selezione dell'Algoritmo**: Lo scheduler utilizza uno degli algoritmi predefiniti per selezionare il prossimo task da eseguire.
3. **Esecuzione del Task**: Il task selezionato viene eseguito, aggiornando le metriche (es. tempo di esecuzione, jitter).
4. **Normalizzazione delle Priorità**: Per lo scheduling basato su priorità, le priorità dinamiche vengono resettate periodicamente.

## Come Utilizzarlo
### Configurazione
1. Clona il repository.
2. Configura l'hardware in `hardware_cfg.h`.
3. Registra le funzioni di inizializzazione nei tuoi moduli usando `initcalls`.
4. Aggiungi i task allo scheduler in `main.c`.

### Interazione tramite Terminale
- Usa un emulatore di terminale compatibile VT100 per connetterti al microcontrollore.
- Esempi di comandi:
  - `HELP`: Mostra l'elenco dei comandi disponibili.
  - `TASK`: Gestisce i task (es. visualizza, pausa, riprendi).
  - `LOGIN <password>`: Autenticati per azioni privilegiate.
  - `REBOOT`: Riavvia il sistema.

## Esempio Pratico
Di seguito un esempio completo per configurare ed eseguire lo scheduler:
```c
#include "scheduler.h"
#include "config.h"

void my_task(void) {
    // Logica del task
    printf("Esecuzione di my_task\n");
}

int main() {
    stdio_init_all();
    select_hardware_config();
    initcalls();
    init_params();

    scheduler_add_task("MyTask", my_task, 1, 1000000, TASK_RUNNING, 0);
    scheduler_run();

    return 0;
}
```

## Aspetti Didattici
- **HAL (Hardware Abstraction Layer)**:
  - Consente la separazione tra hardware specifico e logica applicativa, migliorando la portabilità.
- **Design Modulare**:
  - Ogni modulo ha responsabilità ben definite e interfacce chiare, facilitando la collaborazione e il debugging.
- **Uso di Strutture come Oggetti**:
  - Favorisce la leggibilità e facilita l'estensione del codice, senza la complessità di un linguaggio OOP.
- **Meccanismi di Scheduling**:
  - Introduzione a diversi algoritmi con esempi pratici di utilizzo nei sistemi embedded.

## Estendere lo Scheduler
1. **Aggiungere un Nuovo Algoritmo**:
   - Implementa la logica di selezione in `scheduler_core.c`.
   - Aggiungi la funzione all'array `sched_algorithms`.
2. **Creare un Nuovo Task**:
   - Definisci la logica del task in un nuovo file `.c`.
   - Registra il task usando `scheduler_add_task`.
3. **Migliorare il Terminale**:
   - Registra nuovi comandi in `cmd.c` con descrizioni e gestori dedicati.

## Debugging e Ottimizzazione
- Utilizza `debug.c` per abilitare il logging specifico dei task.
- Analizza le metriche fornite da `scheduler_print_task_list` per identificare colli di bottiglia e ottimizzare l'uso delle risorse.
- Salva e carica configurazioni usando `flash.c` per mantenere parametri tra i riavvii.

## Contribuire
Le contribuzioni sono benvenute! Non ci sono regole, fate una pull request !

## Licenza
Questo progetto è rilasciato sotto la licenza GNU v3. Consulta `LICENSE` per i dettagli.

