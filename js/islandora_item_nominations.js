/**
 * @file
 * only let one checkbox be selected.
 */


(function ($) {
  console.log('ok');
  $('.form-checkbox.nominate').change(function () {
    if (this.click) {
      $('.form-checkbox.nominate').not(this).prop('checked', false);
    }
  });

}(jQuery));

