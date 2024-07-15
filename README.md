# pvDemoAlarm

Para executar este projeto você precisa do [pvbrowser®](https://pvbrowser.de).
Para facilitar a configuração de um setup completo para trabalhar com o pvbrowser em uma distribuição Linux baseada no Ubuntu 24.04, foi criado um script com este propósito:

```
curl jjmg.eng.br/install/setup-full.sh | sudo bash
```
O script intalará todos os pacotes que são usados nos projetos da JJMG:

- pvbrowser®
- MariaDB
- Grafana 11
- ZeroTier
- libpugixml

- Diversas outras bibliotecas de comunicação

- Programas para teste com o protocolo Modbus:
  - qModMaster
  - ScanBus_BR
  - diagslave
  - OpenModScan
  - OpenModSim

Entre na pasta e compile o projeto com:
```
make
g++ modbusdaemon.cpp -o modbusdaemon -I /opt/pvb/rllib/lib/ -lrllib
```
Execute o diagslave na porta 5502
```
diagslave -p 5502
```
Execute o daemon modbus e o pvserver
```
./pvs & ./modbusdaemon
```
Execute o projeto [pvModbusSimulator](https://github.com/jjmg-eng/pvModbusSimulator) para abastecer o diagslave com dados.

Você também pode executar outro script que fará a instalação dos projetos pvDemoAlarm, pvModbusDemo e pvModbusSimulator:
```
curl jjmg.eng.br/install/demo.sh | bash
```
Todos os arquivos estarão na pasta `~/PVprojects`
