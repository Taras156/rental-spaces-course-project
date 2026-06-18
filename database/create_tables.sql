-- Физическая модель базы данных
-- Курсовой проект по БД
-- Предметная область: сдача в аренду торговых площадей
-- СУБД: PostgreSQL

-- Скрипт нужно запускать внутри заранее созданной базы данных, например rental_spaces_db.

CREATE EXTENSION IF NOT EXISTS btree_gist;

DROP TABLE IF EXISTS payments CASCADE;
DROP TABLE IF EXISTS rented_spaces CASCADE;
DROP TABLE IF EXISTS rental_contracts CASCADE;
DROP TABLE IF EXISTS clients CASCADE;
DROP TABLE IF EXISTS users_roles CASCADE;
DROP TABLE IF EXISTS users CASCADE;
DROP TABLE IF EXISTS roles CASCADE;
DROP TABLE IF EXISTS retail_spaces CASCADE;

DROP FUNCTION IF EXISTS check_payment_date_in_rental_period();


-- 1. Роли(id_роли, название, описание)
CREATE TABLE roles (
    id_role INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE,
    description TEXT
);


-- 2. Пользователи(id_пользователя, логин, пароль)
CREATE TABLE users (
    id_user INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    login VARCHAR(100) NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,

    CHECK (length(trim(login)) > 0),
    CHECK (length(password_hash) > 0)
);


-- 3. Пользователи_Роли(id_пользователя, id_роли)
CREATE TABLE users_roles (
    id_user INTEGER NOT NULL,
    id_role INTEGER NOT NULL,

    PRIMARY KEY (id_user, id_role),

    FOREIGN KEY (id_user)
        REFERENCES users(id_user)
        ON UPDATE CASCADE
        ON DELETE CASCADE,

    FOREIGN KEY (id_role)
        REFERENCES roles(id_role)
        ON UPDATE CASCADE
        ON DELETE RESTRICT
);


-- 4. Клиенты(id_клиента, id_пользователя, название, адрес, телефон, реквизиты, контактное_лицо)
CREATE TABLE clients (
    id_client INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    id_user INTEGER NOT NULL UNIQUE,
    name VARCHAR(200) NOT NULL,
    address VARCHAR(300) NOT NULL,
    phone VARCHAR(30) NOT NULL UNIQUE,
    requisites VARCHAR(500) NOT NULL UNIQUE,
    contact_person VARCHAR(150) NOT NULL,

    UNIQUE (name, address),

    FOREIGN KEY (id_user)
        REFERENCES users(id_user)
        ON UPDATE CASCADE
        ON DELETE RESTRICT,

    CHECK (length(trim(name)) > 0),
    CHECK (length(trim(address)) > 0),
    CHECK (length(trim(phone)) > 0),
    CHECK (length(trim(requisites)) > 0),
    CHECK (length(trim(contact_person)) > 0)
);


-- 5. Торговые_точки(id_точки, стоимость_аренды_в_день, кондиционер, площадь, этаж, свободная)
CREATE TABLE retail_spaces (
    id_space INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    rent_price_per_day NUMERIC(12, 2) NOT NULL,
    has_air_conditioner BOOLEAN NOT NULL DEFAULT FALSE,
    area NUMERIC(10, 2) NOT NULL,
    floor_number INTEGER NOT NULL,
    is_available BOOLEAN NOT NULL DEFAULT TRUE,

    CHECK (rent_price_per_day > 0),
    CHECK (area > 0),
    CHECK (floor_number >= 0)
);


-- 6. Договоры_аренды(id_договора, id_клиента, дата_заключения)
CREATE TABLE rental_contracts (
    id_contract INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    id_client INTEGER NOT NULL,
    conclusion_date DATE NOT NULL,

    FOREIGN KEY (id_client)
        REFERENCES clients(id_client)
        ON UPDATE CASCADE
        ON DELETE RESTRICT
);


-- 7. Арендуются(id_договора, id_точки, дата_начала, дата_окончания)
CREATE TABLE rented_spaces (
    id_contract INTEGER NOT NULL,
    id_space INTEGER NOT NULL,
    start_date DATE NOT NULL,
    end_date DATE NOT NULL,

    PRIMARY KEY (id_contract, id_space),

    FOREIGN KEY (id_contract)
        REFERENCES rental_contracts(id_contract)
        ON UPDATE CASCADE
        ON DELETE CASCADE,

    FOREIGN KEY (id_space)
        REFERENCES retail_spaces(id_space)
        ON UPDATE CASCADE
        ON DELETE RESTRICT,

    CHECK (end_date >= start_date),

    -- Одна торговая точка не может быть сдана в аренду на пересекающиеся периоды.
    EXCLUDE USING gist (
        id_space WITH =,
        daterange(start_date, end_date, '[]') WITH &&
    )
);


-- 8. Платежи(id_договора, id_точки, дата_оплаты, сумма_платежа)
CREATE TABLE payments (
    id_contract INTEGER NOT NULL,
    id_space INTEGER NOT NULL,
    payment_date DATE NOT NULL,
    payment_amount NUMERIC(12, 2) NOT NULL,

    PRIMARY KEY (id_contract, id_space, payment_date),

    FOREIGN KEY (id_contract, id_space)
        REFERENCES rented_spaces(id_contract, id_space)
        ON UPDATE CASCADE
        ON DELETE CASCADE,

    CHECK (payment_amount > 0)
);


-- Индексы для внешних ключей
CREATE INDEX idx_users_roles_id_role ON users_roles(id_role);
CREATE INDEX idx_clients_id_user ON clients(id_user);
CREATE INDEX idx_rental_contracts_id_client ON rental_contracts(id_client);
CREATE INDEX idx_rented_spaces_id_space ON rented_spaces(id_space);
CREATE INDEX idx_payments_rented_space ON payments(id_contract, id_space);


-- Триггер: дата платежа должна входить в период аренды соответствующей торговой точки.
CREATE OR REPLACE FUNCTION check_payment_date_in_rental_period()
RETURNS TRIGGER AS $$
DECLARE
    rental_start DATE;
    rental_end DATE;
BEGIN
    SELECT start_date, end_date
    INTO rental_start, rental_end
    FROM rented_spaces
    WHERE id_contract = NEW.id_contract
      AND id_space = NEW.id_space;

    IF NEW.payment_date < rental_start OR NEW.payment_date > rental_end THEN
        RAISE EXCEPTION 'Дата платежа должна находиться в периоде аренды торговой точки';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_check_payment_date_in_rental_period
BEFORE INSERT OR UPDATE ON payments
FOR EACH ROW
EXECUTE FUNCTION check_payment_date_in_rental_period();


-- Тестовые данные для проверки работы скрипта

INSERT INTO roles (name, description) VALUES
('Администратор', 'Пользователь с полным доступом к данным системы'),
('Клиент', 'Пользователь с доступом к собственным договорам и платежам');

INSERT INTO users (login, password_hash) VALUES
('admin', 'Admin_123_hash'),
('client_alpha', 'Client_123_hash'),
('client_beta', 'Client_456_hash');

INSERT INTO users_roles (id_user, id_role) VALUES
(1, 1),
(2, 2),
(3, 2);

INSERT INTO clients (id_user, name, address, phone, requisites, contact_person) VALUES
(2, 'ООО Альфа', 'г. Москва, ул. Центральная, д. 10', '+7-900-111-11-11', 'ИНН 7700000001, КПП 770001001', 'Иванов Иван Иванович'),
(3, 'ООО Бета', 'г. Москва, ул. Северная, д. 5', '+7-900-222-22-22', 'ИНН 7700000002, КПП 770001002', 'Петров Петр Петрович');

INSERT INTO retail_spaces (rent_price_per_day, has_air_conditioner, area, floor_number, is_available) VALUES
(1500.00, TRUE, 25.50, 1, FALSE),
(2100.00, TRUE, 40.00, 2, FALSE),
(1200.00, FALSE, 18.00, 1, TRUE);

INSERT INTO rental_contracts (id_client, conclusion_date) VALUES
(1, '2026-06-01'),
(2, '2026-06-05');

INSERT INTO rented_spaces (id_contract, id_space, start_date, end_date) VALUES
(1, 1, '2026-06-01', '2026-06-30'),
(1, 2, '2026-06-01', '2026-06-30'),
(2, 3, '2026-07-01', '2026-07-31');

INSERT INTO payments (id_contract, id_space, payment_date, payment_amount) VALUES
(1, 1, '2026-06-10', 45000.00),
(1, 2, '2026-06-10', 63000.00),
(2, 3, '2026-07-10', 37200.00);
