-- phpMyAdmin SQL Dump
-- version 3.5.7
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Apr 19, 2013 at 04:05 PM
-- Server version: 5.5.30
-- PHP Version: 5.4.12

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `EventDisplayDB_v042`
--

-- --------------------------------------------------------

--
-- Table structure for table `adPmt`
--

CREATE TABLE IF NOT EXISTS `adPmt` (
  `adPmtId` int(11) NOT NULL AUTO_INCREMENT,
  `hallId` int(11) NOT NULL,
  `detectorId` int(11) NOT NULL,
  `ring` int(11) unsigned NOT NULL,
  `col` int(11) unsigned NOT NULL,
  `centerX` float DEFAULT NULL,
  `centerY` float DEFAULT NULL,
  `centerZ` float DEFAULT NULL,
  `normalX` float DEFAULT NULL,
  `normalY` float DEFAULT NULL,
  `normalZ` float DEFAULT NULL,
  PRIMARY KEY (`adPmtId`),
  UNIQUE KEY `key` (`hallId`,`detectorId`,`ring`,`col`),
  KEY `fk_adPmt_detector1_idx` (`detectorId`,`hallId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `calibAdPmt`
--

CREATE TABLE IF NOT EXISTS `calibAdPmt` (
  `calibAdPmtId` int(11) NOT NULL AUTO_INCREMENT,
  `detectorTriggerId` int(11) NOT NULL,
  `adPmtId` int(11) NOT NULL,
  `totCharge` float DEFAULT NULL,
  PRIMARY KEY (`calibAdPmtId`),
  UNIQUE KEY `key` (`detectorTriggerId`,`adPmtId`),
  KEY `fk_adPmtCalibStats_trigger1_idx` (`detectorTriggerId`),
  KEY `fk_adPmtCalibStats_adPmt1_idx` (`adPmtId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `calibAdPmtTdc`
--

CREATE TABLE IF NOT EXISTS `calibAdPmtTdc` (
  `calibAdPmtTdcId` int(11) NOT NULL AUTO_INCREMENT,
  `calibAdPmtId` int(11) NOT NULL,
  `time_ps` int(11) DEFAULT NULL,
  `charge` float DEFAULT NULL,
  PRIMARY KEY (`calibAdPmtTdcId`),
  UNIQUE KEY `key` (`calibAdPmtId`,`time_ps`),
  KEY `fk_adPmtCalib_adPmtCalibStats1_idx` (`calibAdPmtId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `calibRpcStrip`
--

CREATE TABLE IF NOT EXISTS `calibRpcStrip` (
  `detectorTriggerId` int(11) NOT NULL,
  `rpcStripId` int(11) NOT NULL,
  PRIMARY KEY (`detectorTriggerId`,`rpcStripId`),
  KEY `fk_rpcStripCalib_trigger1_idx` (`detectorTriggerId`),
  KEY `fk_calibRpcStrip_rpcStrip1_idx` (`rpcStripId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `calibWsPmt`
--

CREATE TABLE IF NOT EXISTS `calibWsPmt` (
  `calibWsPmtId` int(11) NOT NULL AUTO_INCREMENT,
  `detectorTriggerId` int(11) NOT NULL,
  `wsPmtId` int(11) NOT NULL,
  `totCharge` float DEFAULT NULL,
  PRIMARY KEY (`calibWsPmtId`),
  UNIQUE KEY `key` (`detectorTriggerId`,`wsPmtId`),
  KEY `fk_wsPmtCalibStats_wsPmt1_idx` (`wsPmtId`),
  KEY `fk_wsPmtCalibStats_trigger1_idx` (`detectorTriggerId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `calibWsPmtTdc`
--

CREATE TABLE IF NOT EXISTS `calibWsPmtTdc` (
  `calibWsPmtTdcId` int(11) NOT NULL AUTO_INCREMENT,
  `calibWsPmtId` int(11) NOT NULL,
  `time_ps` int(11) DEFAULT NULL,
  `charge` float DEFAULT NULL,
  PRIMARY KEY (`calibWsPmtTdcId`),
  UNIQUE KEY `key` (`calibWsPmtId`,`time_ps`),
  KEY `fk_wsPmtCalib_wsPmtCalibStats1_idx` (`calibWsPmtId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `detector`
--

CREATE TABLE IF NOT EXISTS `detector` (
  `detectorId` int(11) NOT NULL,
  `hallId` int(11) NOT NULL,
  `originX` float DEFAULT NULL,
  `originY` float DEFAULT NULL,
  `originZ` float DEFAULT NULL,
  PRIMARY KEY (`detectorId`,`hallId`),
  KEY `fk_detector_hall1_idx` (`hallId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `detectorTrigger`
--

CREATE TABLE IF NOT EXISTS `detectorTrigger` (
  `detectorTriggerId` int(11) NOT NULL AUTO_INCREMENT,
  `hallId` int(11) NOT NULL,
  `detectorId` int(11) NOT NULL,
  `triggerNumber` int(11) NOT NULL,
  `dateNTime` datetime NOT NULL,
  `runNumber` int(11) DEFAULT NULL,
  `triggerTimeSec` int(11) DEFAULT NULL,
  `triggerTimeNanoSec` int(11) DEFAULT NULL,
  `triggerConditionHex` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`detectorTriggerId`),
  UNIQUE KEY `key` (`hallId`,`detectorId`,`triggerNumber`),
  KEY `fk_trigger_detector1_idx` (`detectorId`,`hallId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `hall`
--

CREATE TABLE IF NOT EXISTS `hall` (
  `hallId` int(11) NOT NULL,
  `originX` float DEFAULT NULL,
  `originY` float DEFAULT NULL,
  `originZ` float DEFAULT NULL,
  PRIMARY KEY (`hallId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `muon`
--

CREATE TABLE IF NOT EXISTS `muon` (
  `muonId` int(11) NOT NULL AUTO_INCREMENT,
  `timeStampSec` int(11) NOT NULL,
  `timeStampNanoSec` int(11) NOT NULL,
  `hallId` int(11) NOT NULL,
  `dateNTime` datetime DEFAULT NULL,
  `runNumber` int(11) DEFAULT NULL,
  PRIMARY KEY (`muonId`),
  UNIQUE KEY `key` (`timeStampSec`,`timeStampNanoSec`,`hallId`),
  KEY `fk_muon_hall1_idx` (`hallId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `muonTriggerConstituent`
--

CREATE TABLE IF NOT EXISTS `muonTriggerConstituent` (
  `detectorTriggerId` int(11) NOT NULL,
  `muonId` int(11) NOT NULL,
  `dtMu_ms` float DEFAULT NULL,
  PRIMARY KEY (`detectorTriggerId`,`muonId`),
  KEY `fk_muonTriggerConstituent_trigger1_idx` (`detectorTriggerId`),
  KEY `fk_muonTriggerConstituent_muon1_idx` (`muonId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `recAdSimple`
--

CREATE TABLE IF NOT EXISTS `recAdSimple` (
  `detectorTriggerId` int(11) NOT NULL,
  `energy` float DEFAULT NULL,
  `energyStatus` int(11) DEFAULT NULL,
  `x` float DEFAULT NULL,
  `y` float DEFAULT NULL,
  `z` float DEFAULT NULL,
  `positionStatus` int(11) DEFAULT NULL,
  PRIMARY KEY (`detectorTriggerId`),
  KEY `fk_recAdSimple_detectorTrigger1_idx` (`detectorTriggerId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `recPoolSimple`
--

CREATE TABLE IF NOT EXISTS `recPoolSimple` (
  `detectorTriggerId` int(11) NOT NULL,
  `x` float DEFAULT NULL,
  `y` float DEFAULT NULL,
  `z` float DEFAULT NULL,
  PRIMARY KEY (`detectorTriggerId`),
  KEY `fk_recPoolSimple_detectorTrigger1_idx` (`detectorTriggerId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `recRpcSimple`
--

CREATE TABLE IF NOT EXISTS `recRpcSimple` (
  `detectorTriggerId` int(11) NOT NULL,
  `clusterId` int(11) NOT NULL,
  `x` float DEFAULT NULL,
  `y` float DEFAULT NULL,
  `z` float DEFAULT NULL,
  PRIMARY KEY (`detectorTriggerId`,`clusterId`),
  KEY `fk_recRpcSimple_detectorTrigger1_idx` (`detectorTriggerId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `rpcStrip`
--

CREATE TABLE IF NOT EXISTS `rpcStrip` (
  `rpcStripId` int(11) NOT NULL AUTO_INCREMENT,
  `hallId` int(11) NOT NULL,
  `detectorId` int(11) NOT NULL,
  `row` int(11) NOT NULL,
  `col` int(11) NOT NULL,
  `layer` int(11) NOT NULL,
  `strip` int(11) NOT NULL,
  `centerX` float DEFAULT NULL,
  `centerY` float DEFAULT NULL,
  `centerZ` float DEFAULT NULL,
  `normalX` float DEFAULT NULL,
  `normalY` float DEFAULT NULL,
  `normalZ` float DEFAULT NULL,
  PRIMARY KEY (`rpcStripId`),
  UNIQUE KEY `key` (`hallId`,`detectorId`,`row`,`col`,`layer`,`strip`),
  KEY `fk_rpcStrip_detector1_idx` (`detectorId`,`hallId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `triggerCondition`
--

CREATE TABLE IF NOT EXISTS `triggerCondition` (
  `detectorTriggerId` int(11) NOT NULL,
  `triggerName` varchar(45) NOT NULL,
  PRIMARY KEY (`detectorTriggerId`,`triggerName`),
  KEY `fk_triggerCondition_detectorTrigger1_idx` (`detectorTriggerId`),
  KEY `fk_triggerCondition_triggerType1_idx` (`triggerName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `triggerType`
--

CREATE TABLE IF NOT EXISTS `triggerType` (
  `triggerName` varchar(45) NOT NULL,
  `triggerTypeHex` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`triggerName`),
  UNIQUE KEY `triggerTypeHex` (`triggerTypeHex`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `wsPmt`
--

CREATE TABLE IF NOT EXISTS `wsPmt` (
  `wsPmtId` int(11) NOT NULL AUTO_INCREMENT,
  `hallId` int(11) NOT NULL,
  `detectorId` int(11) NOT NULL,
  `wallNumber` int(11) NOT NULL,
  `wallSpot` int(11) NOT NULL,
  `inwardFacing` tinyint(1) NOT NULL,
  `centerX` float DEFAULT NULL,
  `centerY` float DEFAULT NULL,
  `centerZ` float DEFAULT NULL,
  `normalX` float DEFAULT NULL,
  `normalY` float DEFAULT NULL,
  `normalZ` float DEFAULT NULL,
  PRIMARY KEY (`wsPmtId`),
  UNIQUE KEY `key` (`hallId`,`detectorId`,`wallNumber`,`wallSpot`,`inwardFacing`),
  KEY `fk_wsPmt_detector1_idx` (`detectorId`,`hallId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `adPmt`
--
ALTER TABLE `adPmt`
  ADD CONSTRAINT `fk_adPmt_detector1` FOREIGN KEY (`detectorId`, `hallId`) REFERENCES `detector` (`detectorId`, `hallId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `calibAdPmt`
--
ALTER TABLE `calibAdPmt`
  ADD CONSTRAINT `fk_adPmtCalibStats_adPmt1` FOREIGN KEY (`adPmtId`) REFERENCES `adPmt` (`adPmtId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `fk_adPmtCalibStats_trigger1` FOREIGN KEY (`detectorTriggerId`) REFERENCES `detectorTrigger` (`detectorTriggerId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `calibAdPmtTdc`
--
ALTER TABLE `calibAdPmtTdc`
  ADD CONSTRAINT `fk_adPmtCalib_adPmtCalibStats1` FOREIGN KEY (`calibAdPmtId`) REFERENCES `calibAdPmt` (`calibAdPmtId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `calibRpcStrip`
--
ALTER TABLE `calibRpcStrip`
  ADD CONSTRAINT `fk_calibRpcStrip_rpcStrip1` FOREIGN KEY (`rpcStripId`) REFERENCES `rpcStrip` (`rpcStripId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `fk_rpcStripCalib_trigger1` FOREIGN KEY (`detectorTriggerId`) REFERENCES `detectorTrigger` (`detectorTriggerId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `calibWsPmt`
--
ALTER TABLE `calibWsPmt`
  ADD CONSTRAINT `fk_wsPmtCalibStats_trigger1` FOREIGN KEY (`detectorTriggerId`) REFERENCES `detectorTrigger` (`detectorTriggerId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `fk_wsPmtCalibStats_wsPmt1` FOREIGN KEY (`wsPmtId`) REFERENCES `wsPmt` (`wsPmtId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `calibWsPmtTdc`
--
ALTER TABLE `calibWsPmtTdc`
  ADD CONSTRAINT `fk_wsPmtCalib_wsPmtCalibStats1` FOREIGN KEY (`calibWsPmtId`) REFERENCES `calibWsPmt` (`calibWsPmtId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `detector`
--
ALTER TABLE `detector`
  ADD CONSTRAINT `fk_detector_hall1` FOREIGN KEY (`hallId`) REFERENCES `hall` (`hallId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `detectorTrigger`
--
ALTER TABLE `detectorTrigger`
  ADD CONSTRAINT `fk_trigger_detector1` FOREIGN KEY (`detectorId`, `hallId`) REFERENCES `detector` (`detectorId`, `hallId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `muon`
--
ALTER TABLE `muon`
  ADD CONSTRAINT `fk_muon_hall1` FOREIGN KEY (`hallId`) REFERENCES `hall` (`hallId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `muonTriggerConstituent`
--
ALTER TABLE `muonTriggerConstituent`
  ADD CONSTRAINT `fk_muonTriggerConstituent_muon1` FOREIGN KEY (`muonId`) REFERENCES `muon` (`muonId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `fk_muonTriggerConstituent_trigger1` FOREIGN KEY (`detectorTriggerId`) REFERENCES `detectorTrigger` (`detectorTriggerId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `recAdSimple`
--
ALTER TABLE `recAdSimple`
  ADD CONSTRAINT `fk_recAdSimple_detectorTrigger1` FOREIGN KEY (`detectorTriggerId`) REFERENCES `detectorTrigger` (`detectorTriggerId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `recPoolSimple`
--
ALTER TABLE `recPoolSimple`
  ADD CONSTRAINT `fk_recPoolSimple_detectorTrigger1` FOREIGN KEY (`detectorTriggerId`) REFERENCES `detectorTrigger` (`detectorTriggerId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `recRpcSimple`
--
ALTER TABLE `recRpcSimple`
  ADD CONSTRAINT `fk_recRpcSimple_detectorTrigger1` FOREIGN KEY (`detectorTriggerId`) REFERENCES `detectorTrigger` (`detectorTriggerId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `rpcStrip`
--
ALTER TABLE `rpcStrip`
  ADD CONSTRAINT `fk_rpcStrip_detector1` FOREIGN KEY (`detectorId`, `hallId`) REFERENCES `detector` (`detectorId`, `hallId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `triggerCondition`
--
ALTER TABLE `triggerCondition`
  ADD CONSTRAINT `fk_triggerCondition_detectorTrigger1` FOREIGN KEY (`detectorTriggerId`) REFERENCES `detectorTrigger` (`detectorTriggerId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `fk_triggerCondition_triggerType1` FOREIGN KEY (`triggerName`) REFERENCES `triggerType` (`triggerName`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `wsPmt`
--
ALTER TABLE `wsPmt`
  ADD CONSTRAINT `fk_wsPmt_detector1` FOREIGN KEY (`detectorId`, `hallId`) REFERENCES `detector` (`detectorId`, `hallId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
