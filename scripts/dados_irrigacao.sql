CREATE TABLE dados_irrigacao (
    id_coleta NUMBER PRIMARY KEY,
    sensor VARCHAR2(50),
    valor_coleta NUMBER,
    data_hora_coleta TIMESTAMP,
    status_rele NUMBER(1),
    motivo_acionamento VARCHAR2(255)
);