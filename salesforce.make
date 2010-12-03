; $Id$
; Salesforce API drush make file

core = 6.x
api = 2

; Grab the Salesforce PHP Toolkit from github
libraries[salesforce][download][type] = git
libraries[salesforce][download][url] = git://github.com/messageagency/salesforce.git