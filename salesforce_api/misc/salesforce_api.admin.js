(function ($) {

Drupal.behaviors.sf_fieldmap_options = {
  attach: function() {
    // Bind the change action
    $('.sf_fieldmap_options').bind('change', function() {
      Drupal.sf_fieldmap_option_change(this);
    });

    // Hide hidden fields
    $(".fieldmap-extra-text").hide();

    // Show any text fields that should be shown
    $('.sf_fieldmap_options').each(function() {
      $(this).trigger('change');
    });
  }
}

Drupal.sf_fieldmap_option_change = function(context) {
  var key = $(context).attr('id');
  key = key.replace(/sf-fieldmap-option-/, '');
  Drupal.sf_fieldmap_option_toggle(context, key);
};

Drupal.sf_fieldmap_option_toggle = function(context, fieldname) {
  var fielddiv = '#' + fieldname + '-extra-hidden';
  if ($(context).val() == 'fixed'
   || $(context).val() == 'php'
   || $(context).val() == 'tokens')  {
    $(fielddiv).show();
  } else  {
    $(fielddiv).hide();
    $(fielddiv + ' input').val('');
  }
  if ($(context).val() != 'php') {
    $(fielddiv + ' .description').hide();
  }
  else {
    $(fielddiv + ' .description').show();
  }
};

})(jQuery);
