CREATE TABLE `localdb`.`WSDATA` 
(
 `TIME` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ,
 `SPEED` DECIMAL(4,2) UNSIGNED NULL DEFAULT '0' ,
 `DIRECTION` INT UNSIGNED NOT NULL DEFAULT '0' ,
 `TEMPERATURE` DECIMAL(3,1) NOT NULL DEFAULT '0' ,
 PRIMARY KEY (`TIME`)
 ) 
 ENGINE = InnoDB;