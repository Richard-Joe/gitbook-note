# test
drop database if EXISTS test;

create database test CHARACTER SET utf8 COLLATE utf8_general_ci;

use test;

CREATE TABLE IF NOT EXISTS `user`(
    `id` INT UNSIGNED AUTO_INCREMENT,
    `name` VARCHAR(100) NOT NULL,
    `age` INT UNSIGNED NOT NULL,
    `reward` INT UNSIGNED NOT NULL,
    PRIMARY KEY ( `id` )
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO user
    (name, age, reward)
    VALUES
    ("lufei", 20, 100000),
    ("suolong", 22, 100000),
    ("xiangjishi", 24, 100000),
    ("kaduo", 49, 1990000),
    ("buluke", 100, 2000),
    ("namei", 18, 50000);

CREATE TABLE IF NOT EXISTS `user`(
    `id` INT UNSIGNED AUTO_INCREMENT,
    `name` VARCHAR(100) NOT NULL,
    `age` INT UNSIGNED NOT NULL,
    `reward` INT UNSIGNED NOT NULL,
    PRIMARY KEY ( `id` )
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

# test1
drop database if EXISTS test1;
create database test1 CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci;
use test1;
CREATE TABLE IF NOT EXISTS `test1_tb` ( `name` VARCHAR(65532) NULL ) ENGINE=InnoDB DEFAULT CHARACTER SET = ascii ROW_FORMAT = COMPACT;