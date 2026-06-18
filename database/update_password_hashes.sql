-- Выполнить после create_tables.sql, чтобы приложение могло входить по нормальным паролям.
-- В приложении пароль хешируется через SHA-256.

UPDATE users
SET password_hash = '61417a3226703cb0bb425b1698edf9f5d32057dd68c324e3cab3798678a0e682'
WHERE login = 'admin';

UPDATE users
SET password_hash = '5d859d4c105510386487fc7d6e48e99d780050fb5f24fe58e8950d75a4919e56'
WHERE login = 'client_alpha';

UPDATE users
SET password_hash = 'd9bcec83ffc3c63c66db8cfdf6905c6bab974671e2f18f83350826c4e8a07dab'
WHERE login = 'client_beta';
