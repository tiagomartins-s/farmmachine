# Sistema de Irrigação Inteligente - Gestão Agrícola

Este projeto consiste em um sistema de monitoramento e controle de irrigação utilizando sensores de temperatura, umidade e luminosidade, com integração de dados do sensor com uma base de dados Oracle e visualizações de análises climáticas obtidas da API Open-Meteo. A interface do sistema foi desenvolvida com Streamlit para facilitar o uso por gestores agrícolas.

## Funcionalidades

- **Coleta de dados de sensores**: Monitoramento de pH, temperatura e umidade em intervalos definidos.
- **Controle automático do relé**: Acionamento do relé baseado em condições predefinidas.
- **Visualização de dados**: Gráficos de temperatura e umidade coletados dos sensores e da API Open-Meteo.
- **Operações CRUD**: Inserir, ler, atualizar e excluir dados de sensores diretamente no banco de dados Oracle.

## Tecnologias Utilizadas

- **C**: Código para controle dos sensores e do relé.
- **Python**: Back-end do sistema de gestão de dados com Streamlit.
- **Oracle Database**: Armazenamento de dados coletados dos sensores.
- **API Open-Meteo**: Obtenção de dados climáticos externos para análise e comparação.
- **Streamlit**: Interface do sistema para facilitar a visualização e operação.

## Pré-requisitos

- **Banco de Dados Oracle**: Configurado com a tabela `dados_irrigacao` (crie a tabela usando o script `.sql` na pasta `scripts`).
- **Python 3.8 ou superior**.
- **Bibliotecas Python**: Instale com `pip install` (veja detalhes abaixo).

## Estrutura do Projeto

- `src/main.py`: Código principal da aplicação em Streamlit.
- `src/sketch.ino`: Código em c com o diagrama de circuito.
- `scripts/`: Pasta contendo o script `.sql` para criação da tabela no banco de dados Oracle.
- `README.md`: Este arquivo, com as instruções de uso do projeto.

## Execução do Projeto

### Preparação do Ambiente

1. **Criação da Tabela no Banco de Dados**  
   Execute o script `.sql` disponível na pasta `scripts` para criar a tabela necessária no banco de dados Oracle.

2. **Configuração do Banco de Dados**  
   Atualize as credenciais do banco de dados na função `conectar_bd` do arquivo `app.py`:
   `def conectar_bd():
       conn = oracledb.connect(
           user="SEU_USUARIO",  # Usuário do banco de dados
           password="SUA_SENHA",  # Senha do banco de dados
           dsn="SEU_DSN"  # Data Source Name (ex.: host:porta/serviço)
       )`

3. **Atualização do URL do Banco de Dados para SQLAlchemy**  
   Na função `inserir_dados`, atualize a URL de conexão do banco de dados:
   `engine = create_engine("oracle+oracledb://SEU_USUARIO:SUA_SENHA@SEU_DSN")`
   
4. **Instalação das Dependências**  
   Execute o comando abaixo para instalar todas as bibliotecas necessárias:
   `pip install streamlit pandas oracledb SQLAlchemy requests matplotlib openpyxl`

4. **Instalação das Dependências**  
   Execute o comando abaixo para instalar todas as bibliotecas necessárias:
   `streamlit run app.py`

## Diagrama de circuito
![Esquema de Conexão dos Sensores](imagens/diagrama-circuito.png)
