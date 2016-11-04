What is this module?
--------------------

The Salesforce Testing module aids in testing the Salesforce integration by creating and saving mock http responses.
This allows Salesforce integration behavior to be testable without requiring external queries to a Salesforce instance.

+++ To create a mock +++

1) Enable this module.

2) Set the variable "drupal_http_request_function" to "salesforce_testing_rest_http_test_generate_mocks_function". Do
   this with drush: drush vset drupal_http_request_function=salesforce_testing_rest_http_test_generate_mocks_function

3) Run a Salesforce sync as necessary.

A new mock should appear in the /mocks directory. Mock filenames are based on the original request's path, uglified via
sha1() for the purpose of keeping the file names short. The original path requested is saved in the mock under the key
"original_string".

Make sure to disable this module when you're finished creating mocks.