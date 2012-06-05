<?php

/**
 * Entity class representing the commerce_discount entity type.
 */
class CommerceDiscount extends Entity {

  /**
   * The machine name of the discount offer.
   *
   * @var string
   */
  public $name;

  /**
   * The label of the discount.
   *
   * @var string
   */
  public $label;

  /**
   * Set the default discount type (bundle).
   *
   * @var string
   */
  public $type = 'order_discount';

  /**
   * Whether the discount is enabled.
   *
   * @var bool
   */
  public $enabled = TRUE;

  public function __construct($values = array()) {
    parent::__construct($values, 'commerce_discount');
  }

  protected function defaultUri() {
    $entity_info = entity_get_info('commerce_discount');
    $path = $entity_info['admin ui']['path'] . '/manage';
    return array('path' => $path . '/' . $this->name);
  }
}


class CommerceDiscountControllerExportable extends EntityAPIControllerExportable {

  /**
   * Override export, to allow embedding the commerce discount offer
   * inside the export of the comemrce discount.
   */
  public function export($entity, $prefix = '') {
    $vars = get_object_vars($entity);

    // Add commerce_discout_offer full entity export.
    $wrapper = entity_metadata_wrapper('commerce_discount', $entity);
    $discount_offer = $wrapper->commerce_discount_offer->value();
    unset($discount_offer->discount_offer_id);
    $vars['commerce_discount_offer'] = get_object_vars($discount_offer);

    unset($vars[$this->statusKey], $vars[$this->moduleKey], $vars['is_new']);
    if ($this->nameKey != $this->idKey) {
      unset($vars[$this->idKey]);
    }
    return entity_var_json_export($vars, $prefix);
  }

  /**
   * Overridden to handle embedded dicount_offer update.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    // If overwriting an existing discount, delete the referenced
    // discount_offer.
    $import_wrapper = entity_metadata_wrapper('commerce_discount', $entity);
    if (!empty($entity->discount_id) && !$import_wrapper->commerce_discount_offer->value()) {
      $discount = entity_load_single('commerce_discount', $entity->discount_id);
      $wrapper = entity_metadata_wrapper('commerce_discount', $discount);
      entity_delete('commerce_discount_offer', $wrapper->commerce_discount_offer->getIdentifier());
    }

    // In case there's imported data of discount offer
    // $wrapper->commerce_discount_offer is empty. If there's discount_offer to
    // import or it was deleted above, recreate it and attach to the discount.
    if (!$import_wrapper->commerce_discount_offer->value()) {
      $discount_offer = entity_create('commerce_discount_offer', $entity->commerce_discount_offer);
      $discount_offer->save();
      $wrapper->commerce_discount_offer->set($discount_offer);
      $wrapper->save();
    }

    return parent::save($entity, $transaction);
  }
}
