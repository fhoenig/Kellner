
-- schema

CREATE TABLE `user` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(32) NOT NULL,
  `password` varchar(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `poll` (
  `id` int(11) NOT NULL auto_increment,
  `question` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `poll_option` (
  `id` int(11) NOT NULL auto_increment,
  `poll_id` int(11) NOT NULL,
  `key` varchar(16) NOT NULL,
  `text` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `poll_choice` (
  `id` int(11) NOT NULL auto_increment,
  `user_id` int(11) NOT NULL,
  `poll_option_id` int(11) NOT NULL,
  `vote_time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  USING BTREE (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- sample user and poll

INSERT INTO `user` (`name`, `password`) VALUES ('test', md5('test'));

INSERT INTO `poll` (`id`, `question`) VALUES (1, 'What framework do you prefer?');
INSERT INTO `poll_option` (`poll_id`, `key`, `text`) VALUES(1, 'a', 'Zend Framework');
INSERT INTO `poll_option` (`poll_id`, `key`, `text`) VALUES(1, 'b', 'Symphony');
INSERT INTO `poll_option` (`poll_id`, `key`, `text`) VALUES(1, 'c', 'I just write code');

