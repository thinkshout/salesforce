(function ($) {
  Drupal.behaviors.salesforceMappingAdmin = {
    attach: function(context) {
      if ($('a#salesforce-field-mappings-reset-key').length) {
        $('a#salesforce-field-mappings-reset-key').once('sf-reset-key', function() {
          $('a#salesforce-field-mappings-reset-key').click(function() {
            $('input[name="key"]').removeAttr('checked');
          });
        })
      }
    }
  }
})(jQuery);