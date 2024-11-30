# RT
 
## Installare esptool su macOS

Per installare `esptool` su macOS, puoi utilizzare `pip`, il gestore di pacchetti per Python. Segui questi passaggi:

### Step 1: Assicurati di avere Python e pip installati

1. **Verifica se Python è installato**:
   ```sh
   python3 --version
   ```
   Se non è installato, puoi scaricarlo da [python.org/downloads](https://www.python.org/downloads/) oppure utilizzare `brew`:
   ```sh
   brew install python
   ```

2. **Verifica se pip è installato**:
   ```sh
   pip3 --version
   ```
   `pip` dovrebbe essere installato automaticamente con Python.

### Step 2: Installa esptool

Una volta che hai Python e pip pronti, esegui il seguente comando per installare `esptool`:

```sh
pip3 install esptool
```

### Step 3: Verifica l'installazione

Dopo aver installato `esptool`, puoi verificarne l'installazione eseguendo:

```sh
esptool.py --version
```

Se tutto è andato bene, vedrai la versione di `esptool` installata.

### Note
- Se hai bisogno dei permessi di amministratore, aggiungi `sudo` al comando:
  ```sh
  sudo pip3 install esptool
  ```
- Puoi anche utilizzare `brew` per installare Python se non lo hai già:
  ```sh
  brew install python
  ```

  
## Per programmare l ESP32

Scaricare: https://dl.espressif.com/esp-at/firmwares/esp32c3/ESP32-C3-MINI-1-AT-V3.3.0.0.zip

```sh
~/Library/Python/3.9/bin/esptool.py --chip auto --port /dev/cu.usbserial-14410 --baud 115200 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x0 bootloader/bootloader.bin 0x60000 esp-at.bin 0x8000 partition_table/partition-table.bin 0xd000 ota_data_initial.bin 0x1e000 at_customize.bin 0x1f000 customized_partitions/mfg_nvs.bin
```

Decomprimi il file che trovi sul repo:
`ESP32-C3-MINI-1-AT-V3.3.0.0.zip`