import pandas as pd
import streamlit as st
import oracledb
from sqlalchemy import create_engine
import requests
import matplotlib.pyplot as plt
import os

# Configuração inicial do sistema
st.set_page_config(page_title="Sistema de Irrigação Inteligente - Gestão Agrícola", layout="wide")
st.title("Sistema de Irrigação Inteligente - Gestão Agrícola")


# Função para conectar ao banco de dados Oracle de forma segura
def conectar_bd():
    conn = oracledb.connect(
        user="rm560639",  # Usuário do banco de dados
        password="311299",  # Senha do banco de dados
        dsn="oracle.fiap.com.br:1521/ORCL"  # Data Source Name (ex.: host:porta/serviço)
    )
    return conn


# Função para carregar dados do arquivo Excel
def carregar_dados(arquivo):
    return pd.read_excel(arquivo)


# Função para inserir dados no banco usando SQLAlchemy
def inserir_dados(dados):
    # Renomeando a coluna para ser compatível com o banco de dados
    # dados = dados.rename(columns={"data/hora_coleta": "data_hora_coleta"})
    engine = create_engine("oracle+oracledb://rm560639:311299@oracle.fiap.com.br:1521/ORCL")
    dados.to_sql('dados_irrigacao', con=engine, if_exists='append', index=False)


# CRUD - Funções para Create, Read, Update e Delete
def ler_dados():
    conn = conectar_bd()
    df = pd.read_sql("SELECT * FROM dados_irrigacao", conn)
    conn.close()
    return df


def inserir_novo_registro(dados):
    conn = conectar_bd()
    cursor = conn.cursor()
    cursor.execute(
        "INSERT INTO dados_irrigacao (id_coleta, sensor, valor_coleta, data_hora_coleta, status_rele, motivo_acionamento) VALUES (:1, :2, :3, :4, :5, :6)",
        (dados['id_coleta'], dados['sensor'], dados['valor_coleta'], dados['data_hora_coleta'], dados['status_rele'],
         dados['motivo_acionamento']))
    conn.commit()
    conn.close()


def atualizar_registro(id, novos_dados):
    conn = conectar_bd()
    cursor = conn.cursor()
    cursor.execute("UPDATE dados_irrigacao SET valor_coleta=:1 WHERE id_coleta=:id", (novos_dados['valor_coleta'], id))
    conn.commit()
    conn.close()


def deletar_registro(id):
    conn = conectar_bd()
    cursor = conn.cursor()
    cursor.execute("DELETE FROM dados_irrigacao WHERE id_coleta=:id", {'id': id})
    conn.commit()
    conn.close()


# Função para obter dados climáticos da API Open-Meteo
def obter_dados_api():
    try:
        # URL e parâmetros para a API Open-Meteo
        url = "https://api.open-meteo.com/v1/forecast"
        params = {
            "latitude": -23.5505,  # Latitude de São Paulo
            "longitude": -46.6333,  # Longitude de São Paulo
            "hourly": "temperature_2m,relative_humidity_2m",  # Dados horários de temperatura e umidade
            "timezone": "America/Sao_Paulo"  # Fuso horário de São Paulo
        }

        resposta = requests.get(url, params=params)
        resposta.raise_for_status()
        dados_climaticos = resposta.json()

        horas = dados_climaticos['hourly']['time']
        temperaturas = dados_climaticos['hourly']['temperature_2m']
        umidades = dados_climaticos['hourly']['relative_humidity_2m']

        return {
            "horas": horas,
            "temperaturas": temperaturas,
            "umidades": umidades
        }

    except requests.RequestException as e:
        st.error(f"Erro ao acessar API Open-Meteo: {e}")
        return None

# Menu principal
menu = st.sidebar.selectbox("Menu", ["Importação de Dados", "CRUD e Visualização", "Análises e Gráficos"])

# Menu para Inserir Arquivo
if menu == "Importação de Dados":
    st.subheader("Importar Arquivo de Dados")
    arquivo = st.file_uploader("Envie o arquivo Excel", type=["xlsx"])

    if arquivo:
        dados = carregar_dados(arquivo)
        st.write("Pré-visualização dos Dados:")
        st.write(dados.head())

        if st.button("Inserir Dados no Banco"):
            inserir_dados(dados)
            st.success("Dados inseridos com sucesso!")

# Menu para CRUD e Visualização
elif menu == "CRUD e Visualização":
    st.subheader("Operações CRUD e Visualização")

    if st.button("Ler Dados do Banco"):
        dados_bd = ler_dados()
        st.write(dados_bd)

    with st.form("Atualizar Dados"):
        id_para_atualizar = st.text_input("ID para Atualizar")
        novo_valor = st.number_input("Novo Valor da Coleta", min_value=0.0)
        if st.form_submit_button("Atualizar"):
            atualizar_registro(id_para_atualizar, {'valor_coleta': novo_valor})
            st.success(f"Registro {id_para_atualizar} atualizado com sucesso!")

    with st.form("Deletar Registro"):
        id_para_deletar = st.text_input("ID para Deletar")
        if st.form_submit_button("Deletar"):
            deletar_registro(id_para_deletar)
            st.success(f"Registro {id_para_deletar} deletado com sucesso!")

# Menu para Análises e Gráficos
elif menu == "Análises e Gráficos":
    submenu = st.sidebar.selectbox("Selecione a Visão", ["Dados da API", "Dados do Banco de Dados"])

    if submenu == "Dados da API":
        st.subheader("Visão dos Dados da API (Open-Meteo)")

        dados_api = obter_dados_api()
        if dados_api:
            st.write("Dados da API Open-Meteo:")
            st.write(dados_api)

            # Gráfico de Temperatura da API
            st.write("Temperatura ao longo do tempo (API)")
            fig, ax = plt.subplots()
            ax.plot(dados_api["horas"], dados_api["temperaturas"], label="Temperatura (°C)")
            ax.set_xlabel("Horas")
            ax.set_ylabel("Temperatura (°C)")
            ax.legend()
            st.pyplot(fig)

            # Gráfico de Umidade da API
            st.write("Umidade ao longo do tempo (API)")
            fig, ax = plt.subplots()
            ax.plot(dados_api["horas"], dados_api["umidades"], label="Umidade (%)")
            ax.set_xlabel("Horas")
            ax.set_ylabel("Umidade (%)")
            ax.legend()
            st.pyplot(fig)

            # Gráfico de Correlação entre Temperatura e Umidade (API)
            st.write("Correlação entre Temperatura e Umidade (API)")
            fig, ax = plt.subplots()
            ax.scatter(dados_api["temperaturas"], dados_api["umidades"], alpha=0.6)
            ax.set_xlabel("Temperatura (°C)")
            ax.set_ylabel("Umidade (%)")
            ax.set_title("Temperatura vs. Umidade")
            st.pyplot(fig)
        else:
            st.error("Dados da API Open-Meteo não disponíveis.")

    elif submenu == "Dados do Banco de Dados":
        st.subheader("Visão dos Dados do Banco de Dados")

        dados_bd = ler_dados()
        if not dados_bd.empty:
            st.write("Dados do Banco de Dados:")
            st.write(dados_bd)

            # Separando dados por tipo de sensor para visualizações
            dados_bd['DATA_HORA_COLETA'] = pd.to_datetime(dados_bd['DATA_HORA_COLETA'])
            leituras_temp = dados_bd[dados_bd['SENSOR'] == 'Temperatura'][['DATA_HORA_COLETA', 'VALOR_COLETA']]
            leituras_umidade = dados_bd[dados_bd['SENSOR'] == 'Umidade'][['DATA_HORA_COLETA', 'VALOR_COLETA']]

            # Gráfico de Temperatura (Banco de Dados)
            st.write("Temperatura ao longo do tempo (Banco de Dados)")
            fig, ax = plt.subplots()
            ax.plot(leituras_temp['DATA_HORA_COLETA'], leituras_temp['VALOR_COLETA'], label="Temperatura Sensor (°C)", marker='o')
            ax.set_xlabel("Data e Hora")
            ax.set_ylabel("Temperatura (°C)")
            ax.legend()
            st.pyplot(fig)

            # Gráfico de Umidade (Banco de Dados)
            st.write("Umidade ao longo do tempo (Banco de Dados)")
            fig, ax = plt.subplots()
            ax.plot(leituras_umidade['DATA_HORA_COLETA'], leituras_umidade['VALOR_COLETA'], label="Umidade Sensor (%)", marker='o')
            ax.set_xlabel("Data e Hora")
            ax.set_ylabel("Umidade (%)")
            ax.legend()
            st.pyplot(fig)

            # Gráfico de Correlação entre Temperatura e Umidade (Banco de Dados)
            st.write("Correlação entre Temperatura e Umidade (Banco de Dados)")
            dados_bd_temp_umidade = dados_bd.pivot_table(index='DATA_HORA_COLETA', columns='SENSOR', values='VALOR_COLETA')
            fig, ax = plt.subplots()
            ax.scatter(dados_bd_temp_umidade['Temperatura'], dados_bd_temp_umidade['Umidade'], alpha=0.6)
            ax.set_xlabel("Temperatura Sensor (°C)")
            ax.set_ylabel("Umidade Sensor (%)")
            ax.set_title("Temperatura vs. Umidade (Banco de Dados)")
            st.pyplot(fig)
        else:
            st.error("Nenhum dado disponível no banco de dados.")
