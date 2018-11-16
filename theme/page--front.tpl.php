<?php 
/**
 * @file
 * Alpha's theme implementation to display a single Drupal page.
 */
?>
<?php if (isset($key_image)) : ?>
  <div class='parralax-mirror'>
    <img class='parralax-slider' src='<?php print $key_image['path']?>'></img>
    <?php foreach ($key_image['meta'] as $meta) : ?>
      <div class='pallax-meta'><?php print $meta ?></div>
    <?php endforeach; ?>
  </div>
<?php endif; ?>
<div<?php print $attributes; ?>>
    <?php if (isset($page['header'])) : ?>
    <?php print render($page['header']); ?>
  <?php endif; ?>
  
  <?php if (isset($page['content'])) : ?>
    <?php print render($page['content']); ?>
  <?php endif; ?>  
  
  <?php if (isset($page['footer'])) : ?>
    <?php print render($page['footer']); ?>
  <?php endif; ?>
</div>