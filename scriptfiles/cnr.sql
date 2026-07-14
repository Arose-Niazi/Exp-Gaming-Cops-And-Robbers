-- ============================================================================
--  EXP Gaming Cops And Robbers — MySQL schema (open.mp edition)
-- ============================================================================
--  Reconstructed from the gamemode's queries; the legacy SA-MP snapshot never
--  shipped a schema file. Matches the code as of the open.mp port (column
--  typos fixed there too: Actors.Location, Interiors.CopsWeapons).
--
--  Usage:
--    CREATE DATABASE cnr CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
--    mysql -u <user> -p cnr < scriptfiles/cnr.sql
--
--  Multi-city note: the gamemode selects a city via `#define CITY_` and maps
--  it to per-city tables (LSplayers/LSvehicles). SF/LV are not populated in
--  the code yet — clone the two LS tables as SFplayers/SFvehicles and
--  LVplayers/LVvehicles when those cities go live.
-- ============================================================================

-- Global accounts (one row per player, shared across cities)
CREATE TABLE IF NOT EXISTS `players` (
    `aID`               INT          NOT NULL AUTO_INCREMENT,
    `UserName`          VARCHAR(24)  NOT NULL,
    `Password`          CHAR(128)    NOT NULL,               -- Whirlpool hash (hex)
    `Email`             VARCHAR(128) NOT NULL DEFAULT '',
    `RegisterDate`      VARCHAR(50)  NOT NULL DEFAULT '',
    `Register_Country`  VARCHAR(56)  NOT NULL DEFAULT '',
    `Rank`              INT          NOT NULL DEFAULT 0,     -- 0 player … 6 owner (1 = retired staff)
    `Dj`                TINYINT(1)   NOT NULL DEFAULT 0,
    `Vip`               TINYINT(1)   NOT NULL DEFAULT 0,
    `RegularPlayer`     TINYINT(1)   NOT NULL DEFAULT 0,
    `AutoLogin`         TINYINT(1)   NOT NULL DEFAULT 1,     -- auto-login via IP + serial match
    `HourlyTimeUpdate`  TINYINT(1)   NOT NULL DEFAULT 1,
    `ShowJoinMessages`  TINYINT(1)   NOT NULL DEFAULT 1,
    `ClassMusic`        TINYINT(1)   NOT NULL DEFAULT 1,
    `MenuTD_BG`         INT          NOT NULL DEFAULT -171,  -- 0xFFFFFF55 as signed 32-bit
    `Online`            TINYINT(1)   NOT NULL DEFAULT 0,
    `LastOnline`        DATETIME     NULL,
    `Latest_IP`         VARCHAR(16)  NOT NULL DEFAULT '',
    `Latest_Serial`     VARCHAR(256) NOT NULL DEFAULT '',
    PRIMARY KEY (`aID`),
    UNIQUE KEY `UserName` (`UserName`)
) ENGINE=InnoDB;

-- Per-city player data (Los Santos)
CREATE TABLE IF NOT EXISTS `LSplayers` (
    `aID`             INT           NOT NULL,
    `Money`           INT           NOT NULL DEFAULT 50000,
    `ClassID`         INT           NOT NULL DEFAULT 0,
    `Skin`            INT           NOT NULL DEFAULT 0,
    `Gender`          INT           NOT NULL DEFAULT 0,      -- 0 male / 1 female / 2 other
    `GenderChanges`   VARCHAR(30)   NOT NULL DEFAULT '',     -- '|'-delimited history
    `SkinsSelected`   TEXT          NULL,                    -- '|'-delimited, up to 270 entries
    `Spawns`          INT           NOT NULL DEFAULT 0,
    `MonthlyActivity` INT           NOT NULL DEFAULT 0,
    `Times_GPSUsed`   INT           NOT NULL DEFAULT 0,
    `AdminJailUntil`  INT           NOT NULL DEFAULT 0,      -- M2: unix time admin-jail ends (0 = free)
    `Wanted`          INT           NOT NULL DEFAULT 0,      -- M3: wanted level 0-10 (§2.1)
    -- M3 (stage 2): cop rewards + jail persistence (design §3)
    `Score`           INT           NOT NULL DEFAULT 0,      -- player score (cop arrests/tickets etc.)
    `CopRank`         INT           NOT NULL DEFAULT 0,      -- cop rank ladder 0-10 Recruit->Commissioner
    `RefillPoints`    INT           NOT NULL DEFAULT 0,      -- M3: cop /refill points (§3.1)
    `SeriousCrimes`   INT           NOT NULL DEFAULT 0,      -- violent/murder charge count (blocks appeal)
    `Jailed`          TINYINT(1)    NOT NULL DEFAULT 0,      -- currently cop-jailed
    `JailUntil`       INT           NOT NULL DEFAULT 0,      -- unix time the cop-jail sentence ends (0 = free)
    `Bail`            INT           NOT NULL DEFAULT 0,      -- bail cost to buy out of jail
    `JailReason`      VARCHAR(64)   NOT NULL DEFAULT '',     -- charge text
    `EscapeChained`   TINYINT(1)    NOT NULL DEFAULT 0,      -- re-caught escapee, cannot /escape again
    -- M3 (stage 3): player crime commands (design §4)
    `Skill`           INT           NOT NULL DEFAULT 0,      -- civ/cop skill (pickpocket/con-artist/rapist/…) set at City Hall (M6)
    `Drugs`           INT           NOT NULL DEFAULT 0,      -- grams of drugs carried (§4.3)
    `STDs`            INT           NOT NULL DEFAULT 0,      -- STD bitmask Chlamydia..Mary Lou (§4.2)
    `Condoms`         INT           NOT NULL DEFAULT 0,      -- condoms carried (reduce infection chance)
    `ChastityBelt`    TINYINT(1)    NOT NULL DEFAULT 0,      -- prevents being raped, breakable
    -- M4 (stage 1): bank, taxes & insurance — the money core (design §10/§11.2)
    `BankMoney`       INT           NOT NULL DEFAULT 0,      -- banked cash (safe from robbery/medical fees)
    `TaxOwed`         INT           NOT NULL DEFAULT 0,      -- unpaid tax carried forward to the next tax tick
    `LifeInsurance`   INT           NOT NULL DEFAULT 0,      -- active life-insurance policy count (0..3)
    `HealthInsExpiry` INT           NOT NULL DEFAULT 0,      -- unix time health cover lapses (0 = none)
    -- M4 (stage 2): lotto, money events (design §9.5/§11.2)
    `LottoNumber`     INT           NOT NULL DEFAULT 0,      -- lotto number picked for the game-day (0 = no ticket)
    -- M4 (stage 3): mission framework — one compact '|'-delimited column holds the
    -- per-mission last-completion GAME-HOUR stamp (g_GameDayCounter*24+GameHour),
    -- one value per registered mission, mirroring the SkinsSelected packing (§6).
    `MissionCooldowns` VARCHAR(128) NOT NULL DEFAULT '',     -- '|'-delimited game-hour completion stamps, one per mission
    -- M5 (stage 2): robbery system + crowbar (design §5.7/§5.8/§11.2)
    `CrowbarEquipped` TINYINT(1)    NOT NULL DEFAULT 0,      -- crowbar clothing item equipped (§5.7 — shortens robbery time + raises register take)
    `RobberyHistory`  INT           NOT NULL DEFAULT 0,      -- bitmask ROB_HOLDUP|ROB_BANK|ROB_CASINO|ROB_HOUSE|ROB_SPECIAL — drives crowbar confiscation on arrest
    -- M5 (stage 3): housing system (design §9.1/§11.3)
    `HouseSpawnID`    INT           NOT NULL DEFAULT -1,     -- houses.ID this player spawns at (/sethousespawn), -1 = default city spawn
    PRIMARY KEY (`aID`),
    CONSTRAINT `fk_LSplayers_aID` FOREIGN KEY (`aID`)
        REFERENCES `players` (`aID`) ON DELETE CASCADE
) ENGINE=InnoDB;

-- Migration note (existing databases): the M2/M3 columns above are added by
--   ALTER TABLE `LSplayers` ADD `AdminJailUntil` INT NOT NULL DEFAULT 0;
--   ALTER TABLE `LSplayers` ADD `Wanted`         INT NOT NULL DEFAULT 0;
-- M3 (stage 2) cop + jail columns:
--   ALTER TABLE `LSplayers`
--     ADD `Score`         INT         NOT NULL DEFAULT 0,
--     ADD `CopRank`       INT         NOT NULL DEFAULT 0,
--     ADD `RefillPoints`  INT         NOT NULL DEFAULT 0,
--     ADD `SeriousCrimes` INT         NOT NULL DEFAULT 0,
--     ADD `Jailed`        TINYINT(1)  NOT NULL DEFAULT 0,
--     ADD `JailUntil`     INT         NOT NULL DEFAULT 0,
--     ADD `Bail`          INT         NOT NULL DEFAULT 0,
--     ADD `JailReason`    VARCHAR(64) NOT NULL DEFAULT '',
--     ADD `EscapeChained` TINYINT(1)  NOT NULL DEFAULT 0;
-- M3 (stage 3) crime/drug/STD columns:
--   ALTER TABLE `LSplayers`
--     ADD `Skill`        INT        NOT NULL DEFAULT 0,
--     ADD `Drugs`        INT        NOT NULL DEFAULT 0,
--     ADD `STDs`         INT        NOT NULL DEFAULT 0,
--     ADD `Condoms`      INT        NOT NULL DEFAULT 0,
--     ADD `ChastityBelt` TINYINT(1) NOT NULL DEFAULT 0;
-- M4 (stage 1) bank / taxes / insurance columns (the money core, §10/§11.2).
-- `Score` already exists (added by M3 stage 2) — do NOT re-add it here.
--   ALTER TABLE `LSplayers`
--     ADD `BankMoney`       INT NOT NULL DEFAULT 0,
--     ADD `TaxOwed`         INT NOT NULL DEFAULT 0,
--     ADD `LifeInsurance`   INT NOT NULL DEFAULT 0,
--     ADD `HealthInsExpiry` INT NOT NULL DEFAULT 0;
-- M4 (stage 2) lotto column (design §9.5/§11.2):
--   ALTER TABLE `LSplayers`
--     ADD `LottoNumber`     INT NOT NULL DEFAULT 0;
-- M4 (stage 3) mission cooldown column (design §6) — one packed '|'-delimited
-- column holding a game-hour completion stamp per registered mission:
--   ALTER TABLE `LSplayers`
--     ADD `MissionCooldowns` VARCHAR(128) NOT NULL DEFAULT '';
-- M5 (stage 2) robbery system + crowbar columns (design §5.7/§5.8/§11.2):
--   ALTER TABLE `LSplayers`
--     ADD `CrowbarEquipped` TINYINT(1) NOT NULL DEFAULT 0,
--     ADD `RobberyHistory`  INT        NOT NULL DEFAULT 0;
-- M5 (stage 3) housing spawn-point column (design §9.1/§11.3) — the houses.ID
-- the player spawns at (/sethousespawn), -1 = default city spawn:
--   ALTER TABLE `LSplayers`
--     ADD `HouseSpawnID` INT NOT NULL DEFAULT -1;
-- (repeat for SFplayers/LVplayers when those cities go live).

-- Per-city persistent vehicles (Los Santos)
CREATE TABLE IF NOT EXISTS `LSvehicles` (
    `ID`       INT         NOT NULL AUTO_INCREMENT,
    `Name`     VARCHAR(32) NOT NULL DEFAULT '',
    `Location` VARCHAR(50) NOT NULL DEFAULT '',
    `Type`     INT         NOT NULL DEFAULT 0,               -- 0 civilian / 1 public / 2 cop
    `Model`    INT         NOT NULL DEFAULT 0,               -- vehicle model, or class id for dealer stock
    `X`        FLOAT       NOT NULL DEFAULT 0,
    `Y`        FLOAT       NOT NULL DEFAULT 0,
    `Z`        FLOAT       NOT NULL DEFAULT 0,
    `A`        FLOAT       NOT NULL DEFAULT 0,
    `COL1`     INT         NOT NULL DEFAULT -1,
    `COL2`     INT         NOT NULL DEFAULT -1,
    PRIMARY KEY (`ID`)
) ENGINE=InnoDB;

-- Player-owned houses (M5 stage 3 — design §9.1/§11.3). One row per house; the
-- entrance (X/Y/Z) carries a pickup + label + map icon, the interior teleports
-- into a per-house VirtualWorld. OwnerAID = players.aID (0/-1 = unowned). Money +
-- a simple item count are stored here; the house robbery MOVES StorageMoney into
-- the robber's hand (no minting). LastVisited drives the 2-week inactivity decay.
CREATE TABLE IF NOT EXISTS `houses` (
    `ID`           INT        NOT NULL AUTO_INCREMENT,
    `OwnerAID`     INT        NOT NULL DEFAULT 0,           -- players.aID of the owner (0 = unowned / for sale)
    `Price`        INT        NOT NULL DEFAULT 250000,      -- current market price (bumps on each sale)
    `X`            FLOAT      NOT NULL DEFAULT 0,            -- entrance world position
    `Y`            FLOAT      NOT NULL DEFAULT 0,
    `Z`            FLOAT      NOT NULL DEFAULT 0,
    `Interior`     INT        NOT NULL DEFAULT 0,           -- SA-MP interior id of the inside
    `VirtualWorld` INT        NOT NULL DEFAULT 0,           -- assigned per-house VW (HOUSE_VW_BASE + index at load)
    `IntX`         FLOAT      NOT NULL DEFAULT 0,           -- interior spawn position (inside)
    `IntY`         FLOAT      NOT NULL DEFAULT 0,
    `IntZ`         FLOAT      NOT NULL DEFAULT 0,
    `ForSale`      TINYINT(1) NOT NULL DEFAULT 1,           -- 1 = purchasable (unowned or owner-listed)
    `Rent`         INT        NOT NULL DEFAULT 0,           -- per-game-day rent set by the owner (0 = not for rent)
    `RenterAID`    INT        NOT NULL DEFAULT 0,           -- players.aID of the current tenant (0 = none) [M5 minimal single-renter]
    `SuperLock`    TINYINT(1) NOT NULL DEFAULT 0,           -- super-lock upgrade fitted (harder to rob, §5.5)
    `Locked`       TINYINT(1) NOT NULL DEFAULT 0,           -- owner-set house lock (blocks entry, distinct from vehicle lock, §7)
    `HasPet`       TINYINT(1) NOT NULL DEFAULT 0,           -- a house pet defends against break-ins (§5.5)
    `StorageMoney` INT        NOT NULL DEFAULT 0,           -- money stored in the house (dodges wealth tax; lootable in a rob)
    `StorageItems` INT        NOT NULL DEFAULT 0,           -- simple stored-item count [M5 minimal; full item stash lands M6]
    `StorageJSON`  TEXT       NULL,                         -- reserved for the full per-item stash (M6, §5.5)
    `LastVisited`  DATETIME   NULL DEFAULT NULL,            -- last time the owner visited (2-week inactivity decay, §3.2)
    PRIMARY KEY (`ID`)
) ENGINE=InnoDB;

-- Server-wide statistics (keyed by STATS_VERSION)
CREATE TABLE IF NOT EXISTS `server_data` (
    `Version`        INT NOT NULL,
    `WeeksCompleted` INT NOT NULL DEFAULT 0,
    `Times_GPSUsed`  INT NOT NULL DEFAULT 0,
    -- M4 (stage 2): accumulating lotto jackpot (design §9.5/§11.4)
    `LottoJackpot`   INT NOT NULL DEFAULT 1000000,
    -- M5 (stage 2): per-branch bank-robbery cooldown (design §5.3/§5.8/§11.4) —
    -- the datetime of the last successful rob on each city branch; the branch is
    -- "closed" until BankRobLast* + the cooldown window. NULL = never robbed.
    `BankRobLastLS`  DATETIME NULL DEFAULT NULL,
    `BankRobLastSF`  DATETIME NULL DEFAULT NULL,
    `BankRobLastLV`  DATETIME NULL DEFAULT NULL,
    PRIMARY KEY (`Version`)
) ENGINE=InnoDB;

-- Migration note (existing databases): the M4 (stage 2) lotto jackpot column
-- (design §11.4).
--   ALTER TABLE `server_data`
--     ADD `LottoJackpot` INT NOT NULL DEFAULT 1000000;
-- M5 (stage 2) per-branch bank-robbery cooldown columns (design §5.3/§5.8/§11.4).
-- Read as unix seconds via UNIX_TIMESTAMP() and written via FROM_UNIXTIME():
--   ALTER TABLE `server_data`
--     ADD `BankRobLastLS` DATETIME NULL DEFAULT NULL,
--     ADD `BankRobLastSF` DATETIME NULL DEFAULT NULL,
--     ADD `BankRobLastLV` DATETIME NULL DEFAULT NULL;

-- Streamed interiors / teleports (admin-built via /addinterior)
CREATE TABLE IF NOT EXISTS `Interiors` (
    `ID`           INT         NOT NULL AUTO_INCREMENT,
    `Type`         INT         NOT NULL DEFAULT 0,           -- simple/regular/staff/cops/teleport/robbery/police
    `Interior`     INT         NOT NULL DEFAULT 0,
    `VirtualWorld` INT         NOT NULL DEFAULT 0,
    `InteriorName` VARCHAR(50) NOT NULL DEFAULT '',
    `OutsideZone`  VARCHAR(50) NOT NULL DEFAULT '',
    `InSpawnX`     FLOAT       NOT NULL DEFAULT 0,
    `InSpawnY`     FLOAT       NOT NULL DEFAULT 0,
    `InSpawnZ`     FLOAT       NOT NULL DEFAULT 0,
    `InSpawnA`     FLOAT       NOT NULL DEFAULT 0,
    `OutSpawnX`    FLOAT       NOT NULL DEFAULT 0,
    `OutSpawnY`    FLOAT       NOT NULL DEFAULT 0,
    `OutSpawnZ`    FLOAT       NOT NULL DEFAULT 0,
    `OutSpawnA`    FLOAT       NOT NULL DEFAULT 0,
    `InPickupX`    FLOAT       NOT NULL DEFAULT 0,
    `InPickupY`    FLOAT       NOT NULL DEFAULT 0,
    `InPickupZ`    FLOAT       NOT NULL DEFAULT 0,
    `OutPickupX`   FLOAT       NOT NULL DEFAULT 0,
    `OutPickupY`   FLOAT       NOT NULL DEFAULT 0,
    `OutPickupZ`   FLOAT       NOT NULL DEFAULT 0,
    `CivWeapons`   TINYINT(1)  NOT NULL DEFAULT 0,
    `CopsWeapons`  TINYINT(1)  NOT NULL DEFAULT 0,
    `Weapons`      TINYINT(1)  NOT NULL DEFAULT 0,
    `3DLabel`      TINYINT(1)  NOT NULL DEFAULT 1,
    PRIMARY KEY (`ID`)
) ENGINE=InnoDB;

-- Shop/service NPC actors (admin-built via /addactor)
CREATE TABLE IF NOT EXISTS `Actors` (
    `ID`            INT         NOT NULL AUTO_INCREMENT,
    `Type`          INT         NOT NULL DEFAULT 0,          -- bank, 24-7, ammunation, … (26 types)
    `Location`      VARCHAR(50) NOT NULL DEFAULT '',
    `VirtualWorld`  INT         NOT NULL DEFAULT 0,
    `Interior`      INT         NOT NULL DEFAULT 0,
    `SpawnX`        FLOAT       NOT NULL DEFAULT 0,
    `SpawnY`        FLOAT       NOT NULL DEFAULT 0,
    `SpawnZ`        FLOAT       NOT NULL DEFAULT 0,
    `SpawnA`        FLOAT       NOT NULL DEFAULT 0,
    `AnimationType` INT         NOT NULL DEFAULT 0,
    `Skin`          INT         NOT NULL DEFAULT 0,
    `3DLabel`       TINYINT(1)  NOT NULL DEFAULT 1,
    PRIMARY KEY (`ID`)
) ENGINE=InnoDB;

-- Native ban system (M2 — replaces the legacy Bans filterscript / shared MG-MM
-- ban DB). One row per (Type, Value): a full ban writes three rows (name, IP,
-- serial). CallForChecking (login_register.inc) SELECTs an active, unexpired
-- match on connect; the admin commands live in systems/bans.inc.
CREATE TABLE IF NOT EXISTS `bans` (
    `BanID`      INT          NOT NULL AUTO_INCREMENT,
    `Type`       INT          NOT NULL DEFAULT 0,            -- 0 name / 1 IP / 2 serial
    `Value`      VARCHAR(256) NOT NULL DEFAULT '',           -- the banned name/IP/serial
    `Nick`       VARCHAR(24)  NOT NULL DEFAULT '',           -- account nick this ban is logged under
    `AdminName`  VARCHAR(24)  NOT NULL DEFAULT '',
    `Reason`     VARCHAR(128) NOT NULL DEFAULT '',
    `Date`       DATETIME     NULL,
    `Expiry`     DATETIME     NULL,                          -- NULL = permanent
    `Active`     TINYINT(1)   NOT NULL DEFAULT 1,
    PRIMARY KEY (`BanID`),
    KEY `idx_value` (`Value`),
    KEY `idx_nick`  (`Nick`)
) ENGINE=InnoDB;
