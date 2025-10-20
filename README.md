# 🍇 Projeto Vinheria — IoT Cloud FIWARE & Dashboard Web

## 📝 Descrição do Projeto
O projeto **Vinheria IoT** tem como objetivo validar uma **arquitetura IoT completa**, coletando dados de sensores conectados a um **ESP32** (simulado no Wokwi) e enviando essas informações em tempo real para uma **plataforma na nuvem**, hospedada em uma **VM Linux na Azure**, utilizando o protocolo **MQTT** e o **broker Mosquitto**.

Além disso, os dados são exibidos em um **dashboard web**, criado com **Go Fiber**, que se conecta via **WebSocket MQTT** e apresenta, de forma visual e dinâmica:
- Temperatura 🌡️  
- Umidade 💧  
- Luminosidade 💡  
- Nível de alagamento 🌊  

---

## 🏗️ Arquitetura Proposta

### Diagrama (simplificado em texto)
```
             ┌────────────┐
             │   DHT22    │── Temperatura / Umidade
             └─────┬──────┘
                   │
             ┌─────┴─────────┐
             │   LDR /       │── Luminosidade
             │ Potenciômetro │
             └─────┬─────────┘
                   │
             ┌─────┴──────┐
             │ HC-SR04    │── Nível de Alagamento
             └─────┬──────┘
                   │
             ┌─────┴────────┐
             │    ESP32     │── MQTT ──► Mosquitto (Azure VM)
             └──────────────┘
                    │
                    ▼
           ┌────────────────────────────┐
           │  Dashboard Web (Fiber + JS)│
           │  ws://20.48.230.121:9001   │
           └────────────────────────────┘
```



### Explicação
- O **ESP32** coleta os dados de todos os sensores.  
- O **LDR** fornece dados de luminosidade.  
- O **DHT22** fornece temperatura e umidade.  
- O **HC-SR04** mede distância/alagamento.  
- Os dados são enviados ao **broker MQTT Mosquitto**, configurado com TCP e WebSocket (porta 9001).  
- Um **dashboard web** exibe os valores em tempo real, sem necessidade de plugins ou softwares adicionais.  

---

## 📦 Recursos Necessários
- 1x **ESP32**  
- 1x **Sensor DHT22** (temperatura e umidade)  
- 1x **LDR** (luminosidade)  
- 1x **Sensor ultrassônico HC-SR04** (distância/alagamento)  
- Protoboard + jumpers  
- Conexão Wi-Fi  
- Servidor Mosquitto com suporte a WebSocket  

---

## 🚀 Instruções de Uso

1. Monte o circuito conforme o diagrama acima.  
2. Carregue o código no **ESP32** utilizando a **Arduino IDE**.  
3. Configure o Wi-Fi e o broker MQTT no código:

```cpp
const char* SSID = "SEU_WIFI";
const char* PASSWORD = "SUA_SENHA";
const char* BROKER_MQTT = "IP_PUBLICO_DO_BROKER";
 ```
---

### 📊 4. Dashboard Web 
- O dashboard se conecta automaticamente em `ws://20.48.230.121` e exibe os dados.  
- Acesse:..
  ```
  http://20.48.230.121/
 ---

## 👥 Integrantes
- Vítor Silva Borsato RM:561805   
- João Pedro Godinho Passiani RM:561602​
- Gabriel Molinari Droppa RM:562082
- Isabela de Deus RM: 565988

---

## 🎥 Vídeo de Demonstração
👉 [Assista ao vídeo aqui](https://youtu.be/Ta4a7xvd3Ns)

---

## 🎮 Simulador do Wokwi
👉 [Clique aqui para testar](https://wokwi.com/projects/444885984639433729)

---

## 📷 Foto do prototipo
👉 [Clique aqui para ver](/img/wokwi.png)
