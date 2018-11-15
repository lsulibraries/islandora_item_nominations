/**
 * @file
 * only let one checkbox be selected.
 */


(function ($) {
  console.log('ok');
  $('.form-checkbox').change(function () {
    if (this.click) {
      $('.form-checkbox').not(this).prop('checked', false);
    }
  });

}(jQuery));

