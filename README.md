# Item Nominations

This module provides an apparatus for _nominating_ items so that their JPG/TN is available to various contexts.

## Description

### Terms

* **context**: some logical space in which nominated images will be used (collection page, front page)
* **front-page pool**: pool of images available for rotation on the front page
* **key image**: image chosen as the single representative for a given context (ie collection)
* **namespace prefix***: when the `islandora_namespace_homepage` module is enabled, namespace prefix is a context additional to _front_ or _collection_

### Routes

1. The following paths are significant:
  1. `/islandora/object/%pid%/manage/properties`
    * alter the properties form to include a checkbox
  2. `/islandora/object/%collection-pid%/manage/collection/nominations`
    * provides a select select list that allows a collection admin to choose a key image for the collection
  3. `/%ns-prefix%/manage`*
    * when the module `islandora_namespace_homepage` is installed, this route provides a select list that allows an institutional admin to choose a key image to represent the institution
  4. `/admin/islandora/tools/nominations/front`
    * provides a [tableselect](https://api.drupal.org/api/drupal/includes%21form.inc/function/theme_tableselect/7.x) containing all images _eligible_ for the front-page pool

### Schema

* **id**: _int_ PRIMARY_KEY, auto-increment
* **pid**: _string_ islandora/fedora pid
* **dsid**: _string_ datastream ID (JPEG, TN)
* **context_id**: _string_ unique id for the context, like  a pid (ie _my-stuff:collection_)
* **context_type** _string_ collection, cpd, namespace
* **front**: _bool_ whether or not this should be included in front-page pool; default TRUE

### Constraints / Requirements / Rules

1. prefer JPG datastream, use TN as a fallback
2. metadata from MODS will be title, description, creator, date, (cmodel is not part of metadata cmodel but pulled from the RI)
3. `is_eligible()` determines item eligibility; nominate control will only be rendered for items that
  1. are not embargoed
  2. are published (toggle_unpublish module_exists)
  3. do not have generic image/tn (audio, that folder icon)
4. front-page pool will only include pids with high-quality images (no TN)
5. a default will be used as key image on first installation and in cases where no suitable item exists (audio collections)
6. front-page pool membership can be vetoed by privileged users (super admins)

### API
The following functions and return values will be needed:

* `islandora_item_nominations_get_key_image($context)`
  * `$context` _string_ context id ie: collection pid, cpd pid, namespace
  * `return [$pid, $dsid]`
  * check* `module_exists('islandora_namespace_homepage')`
* `islandora_item_nominations_get_frontpage_pool()`
  * `return $pool`
  * `$pool` _array_ set of db records `WHERE front = 1`
* `include/utilities.inc nominate($pid, $context)`
  * should be called when a new item is chosen for the given context
  * `$pid` _string_ dora/fedora PID
  * `$context` _string_ either 'coll' or 'ns'
  * set* the appropriate field to NULL
    * example: `UPDATE <tablename> SET collkey = NULL;`
* `include/utilities.inc nominate($pid, $dsid)`
  * update the db record for a pid, setting a value for a context
  * `$pid` _string_ dora/fedora PID
  * `$dsid` _string_ Datastream ID
* `islandora_item_nominations_get_image_metadata($pid)`
  * return [$title, $description, $creator, $date_created, $cmodel]

### Forms
* item-level nominate checkbox (hook item properties form)
* context-level select list for setting key item (hook other forms)
* front-page pool management form

## * for consideration
* this module is potentially valuable outside of LSU, decouple this from our namespace-prefix machinery
  * instead, let ns-homepage use this module
* break out the table schema
