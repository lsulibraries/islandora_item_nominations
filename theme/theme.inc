<?php 


/**
 * Theme item nomination form.
 *
 * @param array $variables
 *   Variables passed to this theme function.
 *
 * @return string
 *   Markup representing the table for rendering.
 */
function theme_islandora_item_nominations_table(array $variables) {
  // Manually add the table select javascript as we are using a custom table.
  drupal_add_js('misc/tableselect.js');
  //dpm($variables);
  $table = $variables['table'];
  $row_elements = $table['rows'];
  $rows = array();
  foreach (element_children($row_elements) as $key) {
    $columns = array();
    $row_element = $row_elements[$key];
    foreach (element_children($row_element) as $key) {
      $column_element = $row_element[$key];
      $class = "item_nominations_table_${key}";
      $columns[] = array(
        'data' => drupal_render($column_element),
        'class' => isset($cell_element['#attributes']['class']) ? $column_element['#attributes']['class'] : $class,
      );
    }
    $rows[] = $columns;
  }
  $variables = array(
    'header' => $table['#header'],
    'rows' => $rows,
    'attributes' => $table['#attributes'],
    'caption' => NULL,
    'colgroups' => NULL,
    'sticky' => NULL,
    'empty' => t("No items nominated, edit an object's properties to nominate."),
  );
  return theme('table', $variables);
}
