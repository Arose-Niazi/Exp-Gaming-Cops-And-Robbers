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
    `CopChatOn`       TINYINT(1)    NOT NULL DEFAULT 1,      -- Stage B (§3.1): department-channel (/d) + non-critical dispatch receive toggle (1=on)
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
    `MissionCooldowns` VARCHAR(255) NOT NULL DEFAULT '',     -- '|'-delimited game-hour completion stamps, one per mission (widened 128->255 in M6 stage 4: 19 missions x up-to-9-char stamps overruns 128)
    -- M5 (stage 2): robbery system + crowbar (design §5.7/§5.8/§11.2)
    `CrowbarEquipped` TINYINT(1)    NOT NULL DEFAULT 0,      -- crowbar clothing item equipped (§5.7 — shortens robbery time + raises register take)
    `RobberyHistory`  INT           NOT NULL DEFAULT 0,      -- bitmask ROB_HOLDUP|ROB_BANK|ROB_CASINO|ROB_HOUSE|ROB_SPECIAL — drives crowbar confiscation on arrest
    -- M5 (stage 3): housing system (design §9.1/§11.3)
    `HouseSpawnID`    INT           NOT NULL DEFAULT -1,     -- houses.ID this player spawns at (/sethousespawn), -1 = default city spawn
    -- M6 (stage 1): skills, fighting styles & clothes (design §9.8/§9.7/§11.2)
    `FightStyle`      INT           NOT NULL DEFAULT 4,      -- chosen GTA fighting style (SetPlayerFightingStyle) — 4=NORMAL default; set at a gym via /fightstyle (§9.8)
    `HasDrugBag`      TINYINT(1)    NOT NULL DEFAULT 0,      -- drug bag owned — doubles the Drug Dealer carry cap (§4.3/§16 #2), bought via /clotheswear
    -- M6 (stage 2): fishing & farming (design §9.2/§9.3/§11.2)
    `FishPermits`     INT           NOT NULL DEFAULT 0,      -- fishing permits held (carry up to 50, 1 fish/permit, §9.2)
    `HuntPermits`     INT           NOT NULL DEFAULT 0,      -- hunting permits held (carry up to 20, kill deer w/o = +6 wanted, §9.3)
    `DrugSeeds`       INT           NOT NULL DEFAULT 0,      -- drug plant seeds held (carry up to 10, §9.3)
    `HasFishingRod`   TINYINT(1)    NOT NULL DEFAULT 0,      -- fishing rod owned (faster/higher catch chance, §9.2)
    `HasFishCooler`   TINYINT(1)    NOT NULL DEFAULT 0,      -- fish cooler owned (bigger catch capacity, §9.2)
    `FishSalesPermit` TINYINT(1)    NOT NULL DEFAULT 0,      -- fish sales permit (sell fish to players, §9.2)
    -- Cooler is a compact aggregate inventory (no per-fish table): count + total pounds
    -- + total sale value (weight×rarity already folded in). Sold via /fishsellall (§9.2).
    `CoolerCount`     INT           NOT NULL DEFAULT 0,      -- fish currently in the cooler (0..cap)
    `CoolerWeight`    INT           NOT NULL DEFAULT 0,      -- total pounds of fish in the cooler
    `CoolerValue`     INT           NOT NULL DEFAULT 0,      -- accumulated base sale value of the cooler contents ($, pre-market-mult)
    -- Stage A (post-M7): robbery success grind (FEATURE-EXPANSION-PLAN §1.1)
    `SuccessfulRobberies` INT       NOT NULL DEFAULT 0,      -- the grind counter — drives the per-type success % curve (Rob_SuccessChance)
    `FailedRobberies`     INT       NOT NULL DEFAULT 0,      -- informational / anti-farm telemetry (incremented on a failed rob-type crime)
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
--     ADD `MissionCooldowns` VARCHAR(255) NOT NULL DEFAULT '';
-- (M6 stage 4 widened this 128->255 for the full 19-mission roster; existing
--  servers should widen it too:  ALTER TABLE `LSplayers` MODIFY `MissionCooldowns` VARCHAR(255) NOT NULL DEFAULT '';)
-- M5 (stage 2) robbery system + crowbar columns (design §5.7/§5.8/§11.2):
--   ALTER TABLE `LSplayers`
--     ADD `CrowbarEquipped` TINYINT(1) NOT NULL DEFAULT 0,
--     ADD `RobberyHistory`  INT        NOT NULL DEFAULT 0;
-- M5 (stage 3) housing spawn-point column (design §9.1/§11.3) — the houses.ID
-- the player spawns at (/sethousespawn), -1 = default city spawn:
--   ALTER TABLE `LSplayers`
--     ADD `HouseSpawnID` INT NOT NULL DEFAULT -1;
-- M6 (stage 1) skills/fighting-style/clothes columns (design §9.8/§9.7/§11.2):
--   ALTER TABLE `LSplayers`
--     ADD `FightStyle` INT        NOT NULL DEFAULT 4,
--     ADD `HasDrugBag` TINYINT(1) NOT NULL DEFAULT 0;
-- M6 (stage 2) fishing/farming columns (design §9.2/§9.3/§11.2):
--   ALTER TABLE `LSplayers`
--     ADD `FishPermits`     INT        NOT NULL DEFAULT 0,
--     ADD `HuntPermits`     INT        NOT NULL DEFAULT 0,
--     ADD `DrugSeeds`       INT        NOT NULL DEFAULT 0,
--     ADD `HasFishingRod`   TINYINT(1) NOT NULL DEFAULT 0,
--     ADD `HasFishCooler`   TINYINT(1) NOT NULL DEFAULT 0,
--     ADD `FishSalesPermit` TINYINT(1) NOT NULL DEFAULT 0,
--     ADD `CoolerCount`     INT        NOT NULL DEFAULT 0,
--     ADD `CoolerWeight`    INT        NOT NULL DEFAULT 0,
--     ADD `CoolerValue`     INT        NOT NULL DEFAULT 0;
-- Stage A (post-M7) robbery success-grind columns (FEATURE-EXPANSION-PLAN §1.1):
--   ALTER TABLE `LSplayers`
--     ADD `SuccessfulRobberies` INT NOT NULL DEFAULT 0,
--     ADD `FailedRobberies`     INT NOT NULL DEFAULT 0;
-- Stage B (post-M7) cop-chat toggle column (FEATURE-EXPANSION-PLAN §3.1):
--   ALTER TABLE `LSplayers`
--     ADD `CopChatOn` TINYINT(1) NOT NULL DEFAULT 1;
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

-- Drug plants (M6 stage 2 — design §9.3/§11.3). One row per planted drug crop.
-- OwnerAID = players.aID of the planter. Plants grow over ~20 minutes real time
-- from PlantedAt (unix seconds) toward DRUG_PLANT_MAX_GRAMS; Grams is the current
-- yield snapshot. Plants persist across relogs but DO NOT grow while offline — the
-- growth is recomputed from a "grow-seconds accrued" model on the game-clock tick
-- (GrowSecs) so an offline gap does not advance them. Cops/players can destroy a
-- plant (row deleted). Loaded on init, CRUD is async (mysql_pquery).
CREATE TABLE IF NOT EXISTS `plants` (
    `ID`         INT      NOT NULL AUTO_INCREMENT,
    `OwnerAID`   INT      NOT NULL DEFAULT 0,               -- players.aID of the planter
    `X`          FLOAT    NOT NULL DEFAULT 0,               -- world position of the plant
    `Y`          FLOAT    NOT NULL DEFAULT 0,
    `Z`          FLOAT    NOT NULL DEFAULT 0,
    `Grams`      INT      NOT NULL DEFAULT 0,               -- current yield snapshot (0..DRUG_PLANT_MAX_GRAMS)
    `GrowSecs`   INT      NOT NULL DEFAULT 0,               -- grow-seconds accrued (only advances while owner online, §9.3 "no offline growth")
    `Fertilized` TINYINT(1) NOT NULL DEFAULT 0,             -- fertilized (grows faster but attracts deer, §9.3)
    `PlantedAt`  DATETIME NULL DEFAULT NULL,                -- when it was planted (audit)
    PRIMARY KEY (`ID`)
) ENGINE=InnoDB;

-- Migration note (existing databases): the M6 (stage 2) plants table (design §11.3):
--   (run the CREATE TABLE above; no ALTER needed — it is a new table.)

-- ---------------------------------------------------------------------------
-- Stock market (M7 — design §9.4/§10/§11.3, docs/research/economy.md §7)
-- ---------------------------------------------------------------------------
-- The 21-stock market. Prices are DOUBLE (the pool/price math is float, §7.2).
-- One row per company; ID 0..20 mirrors the E_STOCK_* enum (economy_stub.inc).
-- `Pool` is the running reservoir of net economic activity (fed by
-- StockMarket_UpdateEarnings); on the daily tick the new Price is derived from
-- Pool + drift. Seeded on first boot if the table is empty (Stocks_Init).
CREATE TABLE IF NOT EXISTS `stocks` (
    `ID`              INT      NOT NULL,                  -- 0..20 (E_STOCK_* index, stable)
    `Name`            VARCHAR(40) NOT NULL DEFAULT '',    -- display name
    `Price`           DOUBLE   NOT NULL DEFAULT 1.0,      -- current share price
    `Pool`            DOUBLE   NOT NULL DEFAULT 0.0,      -- running activity reservoir (>=0)
    `AvailableShares` INT      NOT NULL DEFAULT 0,        -- shares left to buy from the market
    `IPOPrice`        DOUBLE   NOT NULL DEFAULT 1.0,      -- initial price (the multiplier baseline)
    `IPOShares`       INT      NOT NULL DEFAULT 0,        -- initial float (dilution scaling)
    `MaxShares`       INT      NOT NULL DEFAULT 0,        -- total shares in existence
    PRIMARY KEY (`ID`)
) ENGINE=InnoDB;

-- Per-account share holdings. One row per (account, stock) with a non-zero
-- holding; the row is deleted when the holding hits 0. FK cascades on account
-- delete. aID = players.aID (global account id).
CREATE TABLE IF NOT EXISTS `stock_holdings` (
    `aID`     INT NOT NULL,                              -- players.aID
    `StockID` INT NOT NULL,                              -- stocks.ID (0..20)
    `Shares`  INT NOT NULL DEFAULT 0,                    -- shares held (>0; row removed at 0)
    PRIMARY KEY (`aID`, `StockID`),
    CONSTRAINT `fk_sh_aID` FOREIGN KEY (`aID`) REFERENCES `players` (`aID`) ON DELETE CASCADE
) ENGINE=InnoDB;

-- Rolling price history (last STOCK_HISTORY_DAYS periods) for /markethistory and
-- price-trend views. Written one row per stock per daily tick; `Day` is the
-- monotonic game-day counter (g_GameDayCounter) so trends survive week rollover.
CREATE TABLE IF NOT EXISTS `stock_reports` (
    `StockID` INT    NOT NULL,                           -- stocks.ID
    `Day`     INT    NOT NULL,                           -- game-day counter at report time
    `Price`   DOUBLE NOT NULL DEFAULT 1.0,               -- closing price that period
    PRIMARY KEY (`StockID`, `Day`)
) ENGINE=InnoDB;

-- Migration note (existing databases): the M7 stock tables (design §11.3):
--   (run the three CREATE TABLE above; no ALTER needed — they are new tables.)

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
    -- M7: global prime rate (design §10.1/§11.4). A master economy multiplier
    -- that moves on the daily stock tick; higher prime = lower goods, higher
    -- houses. Clamped in code to [PRIME_RATE_MIN, PRIME_RATE_MAX].
    `PrimeRate`      DOUBLE NOT NULL DEFAULT 1.0,
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
-- M7 prime-rate column (design §10.1/§11.4):
--   ALTER TABLE `server_data`
--     ADD `PrimeRate` DOUBLE NOT NULL DEFAULT 1.0;

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
