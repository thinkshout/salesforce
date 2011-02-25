
Drupal.behaviors.sf_fieldmap_options = function() {
  $.fn.sf_fieldmap_option_change = function() {
    var key = $(this).attr('id');
    key = key.replace(/sf-fieldmap-option-/, '');
    $(this).sf_fieldmap_option_toggle(key);
  }

  $.fn.sf_fieldmap_option_toggle = function(fieldname) { 
    var fielddiv = '#' + fieldname + '-extra-hidden';
    if ($(this).val() == 'fixed'
   || $(this).val() == 'php')  {
      $(fielddiv).show();
    } else  {
      $(fielddiv).hide();
      $(fielddiv + ' input').val('');
    }
  }
  
  // Bind the change action
  $('.sf_fieldmap_options').bind('change', function() {
    $(this).sf_fieldmap_option_change();
  });

  // Hide hidden fields
  $(".fieldmap-extra-text").hide();

  // Show any text fields that should be shown
  $('.sf_fieldmap_options').each(function() {
    $(this).trigger('change');
  });

}
