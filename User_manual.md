# Manuale Utente - Scheduler Framework per RP2040

Benvenuto nel manuale utente per il framework di scheduling progettato per il microcontrollore RP2040. Questo documento ti guiderà attraverso l'installazione, la configurazione, e l'utilizzo del sistema, spiegando come sfruttare al meglio le sue funzionalità e offrendo dettagli aggiuntivi per gli sviluppatori. Approfondimento anche su un eventuale Porting su STM32.

---

## Introduzione
Questo scheduler è progettato per applicazioni embedded su RP2040, consentendo una gestione avanzata dei task tramite molteplici algoritmi di scheduling e un'interfaccia terminale VT100 per il controllo runtime.

**Caratteristiche principali:**
- Gestione di task con priorità e intervalli configurabili.
- Supporto per più algoritmi di scheduling (Round-robin, Priorità, ecc.).
- Meccanismi di logging e debug integrati.
- Persistenza dei parametri tramite memoria flash.
- Modularità per estensioni future e semplicità di manutenzione.

---

## Installazione
### Prerequisiti
- **Toolchain per RP2040**: Installa il toolchain di sviluppo Pico SDK.
- **Hardware**: Assicurati di avere un dispositivo RP2040 con periferiche necessarie (UART, GPIO, ecc.).

### Procedura
1. Clona il repository:
   ```bash
   git clone <url-repository>
   cd <directory-repository>
   ```
2. Configura l'ambiente Pico SDK:
   ```bash
   export PICO_SDK_PATH=/path/to/pico-sdk
   ```
3. Compila il progetto:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```
4. Flash del firmware sul dispositivo:
   ```bash
   cp <binary>.uf2 /media/<user>/RPI-RP2
   ```
5. Verifica la connessione tramite UART con un terminale compatibile VT100.

---

## Porting su Altre Architetture
Il framework è progettato per essere modulare e adattabile, rendendo possibile il porting su altre architetture a microcontrollore, come STM32.

### Passaggi Generali per il Porting
1. **Adattamento del Hardware Abstraction Layer (HAL):**
   - Modifica o implementa `hardware_cfg.h` per adattarlo alle periferiche e pin del nuovo microcontrollore.
   - Aggiorna le funzioni specifiche del microcontrollore, come GPIO, UART e timer.

2. **Sostituzione delle API Tempo-Reali:**
   - Le funzioni temporali (es. `get_absolute_time`, `absolute_time_diff_us`) devono essere sostituite con equivalenti della nuova piattaforma.
   - Esempio su STM32: utilizza `HAL_GetTick()` o timer hardware.

3. **Revisione della Configurazione:**
   - Verifica le macro di configurazione come `RP2040_TOTAL_RAM` e adattale alla memoria disponibile.
   - Configura la dimensione dello stack e il numero massimo di task in base alle risorse hardware.

4. **Verifica della Persistenza dei Dati:**
   - Sostituisci le funzioni di gestione della memoria flash (`flash.h`) con API specifiche del microcontrollore, come quelle fornite da STM32 HAL o driver esterni.

5. **Testing e Debugging:**
   - Utilizza strumenti di debug disponibili per il microcontrollore target (es. STM32CubeIDE per STM32).

6. **Benchmarking:**
   - Usa GPIO per misurare i tempi di esecuzione dei task e confrontare le prestazioni rispetto all’implementazione su RP2040.
   ```c
   void start_benchmark(void) {
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
   }

   void stop_benchmark(void) {
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
   }
   ```

---

## Esempio di Porting su STM32
### Adattamento di `hardware_cfg.h`
Aggiorna i pin e le configurazioni hardware per STM32:
```c
const HardwareConfig hardware_v1 = {
    .led_pin = GPIO_PIN_13,
    .uart0_tx_pin = GPIO_PIN_2,
    .uart0_rx_pin = GPIO_PIN_3,
    .uart0_baud = 115200
};
```

### Sostituzione delle Funzioni Temporali
Esempio di implementazione per STM32 usando `HAL_GetTick`:
```c
uint32_t get_absolute_time(void) {
    return HAL_GetTick(); // Restituisce il tempo in millisecondi
}

uint32_t absolute_time_diff_us(uint32_t start, uint32_t end) {
    return (end - start) * 1000; // Calcola la differenza in microsecondi
}
```

### Configurazione dei Timer Hardware
Utilizza un timer hardware per ottenere misurazioni precise:
```c
void timer_init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    TIM_HandleTypeDef htim2;
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = (SystemCoreClock / 1000000) - 1; // 1 MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;
    HAL_TIM_Base_Init(&htim2);
    HAL_TIM_Base_Start(&htim2);
}

uint32_t get_timer_us(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}
```

### Persistenza dei Dati
Utilizza l'API HAL per accedere alla memoria flash:
```c
int flash_storage_write(const void *data, size_t size) {
    HAL_FLASH_Unlock();
    for (size_t i = 0; i < size; i += sizeof(uint64_t)) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, FLASH_ADDRESS + i, *((uint64_t *)(data + i)));
    }
    HAL_FLASH_Lock();
    return 0;
}

int flash_storage_read(void *buffer, size_t size) {
    memcpy(buffer, (void *)FLASH_ADDRESS, size);
    return 0;
}
```

---

## Testing del Codice
### Aree Critiche da Testare
1. **Scheduler e Algoritmi**:
   - Testa che ogni algoritmo selezioni i task corretti in base alla logica prevista.
   - Verifica che la normalizzazione delle priorità funzioni come atteso.

2. **Gestione del Tempo**:
   - Assicurati che le funzioni temporali forniscano misurazioni accurate.

3. **Persistenza dei Parametri**:
   - Salva e ricarica parametri dalla flash per confermare l'integrità dei dati.

4. **Uso della Memoria**:
   - Monitora lo stack per rilevare eventuali overflow.

### Esempi di Test
#### Test per lo Scheduler
Utilizza task semplici per verificare l'algoritmo PRIORITY:
```c
void high_priority_task(void) {
    printf("High Priority Task\n");
}

void low_priority_task(void) {
    printf("Low Priority Task\n");
}

int main() {
    scheduler_add_task("High", high_priority_task, 10, 1000000, TASK_RUNNING, 0);
    scheduler_add_task("Low", low_priority_task, 1, 1000000, TASK_RUNNING, 0);
    scheduler_run();
    return 0;
}
```

#### Test per la Persistenza
Conferma che i dati salvati siano corretti:
```c
void test_flash_persistence(void) {
    int data_to_save = 42;
    int data_read = 0;
    flash_storage_write(&data_to_save, sizeof(data_to_save));
    flash_storage_read(&data_read, sizeof(data_read));
    printf("Data: %d\n", data_read);
}
```

---

## Conclusione
Questo manuale copre tutti gli aspetti principali per utilizzare, portare e personalizzare lo Scheduler Framework per RP2040 e altre piattaforme come STM32. Se hai domande o suggerimenti, contattaci tramite il repository GitHub.

