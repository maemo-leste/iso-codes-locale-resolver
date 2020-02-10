#include <glib.h>
#include <libxml/xmlreader.h>

#include <locale.h>
#include <libintl.h>
#include <stdlib.h>

#define ISO_CODES_DIR "/share/xml/iso-codes"
#define ISO_3166_XML_PATH ISO_CODES_PREFIX ISO_CODES_DIR "/iso_3166.xml"
#define ISO_639_XML_PATH ISO_CODES_PREFIX ISO_CODES_DIR "/iso_639.xml"

static void
process_iso_3166_node(xmlTextReaderPtr reader, GHashTable *iso_3166)
{
  const xmlChar *name;
  const xmlChar *iso_3166_alpha_2_code = NULL;
  const xmlChar *iso_3166_name = NULL;

  name = xmlTextReaderConstName(reader);

  if (!name || xmlStrcmp(name, BAD_CAST "iso_3166_entry"))
    return;

  while (xmlTextReaderMoveToNextAttribute (reader) == 1)
  {
    if ((name = xmlTextReaderConstName(reader)))
    {
      if (!xmlStrcmp(name, BAD_CAST "alpha_2_code"))
        iso_3166_alpha_2_code = xmlTextReaderConstValue(reader);
      else if (!xmlStrcmp(name, BAD_CAST "name"))
        iso_3166_name = xmlTextReaderConstValue(reader);

      if (iso_3166_alpha_2_code && iso_3166_name)
      {
        g_hash_table_insert(iso_3166,
                            g_strdup((const gchar *)iso_3166_alpha_2_code),
                            g_strdup((const gchar *)iso_3166_name));
        break;
      }
    }
  }
}

static gboolean
parse_iso_3166(GHashTable *iso_3166)
{
  xmlTextReaderPtr reader;
  int ret;

  reader = xmlReaderForFile(ISO_3166_XML_PATH, NULL, 0);

  g_return_val_if_fail (reader != NULL, FALSE);

  while ((ret = xmlTextReaderRead(reader)) == 1)
    process_iso_3166_node(reader, iso_3166);

  xmlFreeTextReader(reader);

  g_return_val_if_fail (ret == 0, FALSE);

  return TRUE;
}

static void
process_iso_639_node(xmlTextReaderPtr reader, GHashTable *iso_639)
{
  const xmlChar *name;
  const xmlChar *iso_639_1_code = NULL;
  const xmlChar *iso_639_name = NULL;

  name = xmlTextReaderConstName(reader);

  if (!name || xmlStrcmp(name, BAD_CAST "iso_639_entry"))
    return;

  while (xmlTextReaderMoveToNextAttribute (reader) == 1)
  {
    if ((name = xmlTextReaderConstName(reader)))
    {
      if (!xmlStrcmp(name, BAD_CAST "iso_639_1_code"))
        iso_639_1_code = xmlTextReaderConstValue(reader);
      else if (!xmlStrcmp(name, BAD_CAST "name"))
        iso_639_name = xmlTextReaderConstValue(reader);

      if (iso_639_1_code && iso_639_name)
      {
        g_hash_table_insert(iso_639,
                            g_strdup((const gchar *)iso_639_1_code),
                            g_strdup((const gchar *)iso_639_name));
        break;
      }
    }
  }
}

static gboolean
parse_iso_639(GHashTable *iso_639)
{
  xmlTextReaderPtr reader;
  int ret;

  reader = xmlReaderForFile(ISO_639_XML_PATH, NULL, 0);

  g_return_val_if_fail (reader != NULL, FALSE);

  while ((ret = xmlTextReaderRead(reader)) == 1)
    process_iso_639_node(reader, iso_639);

  xmlFreeTextReader(reader);

  g_return_val_if_fail (ret == 0, FALSE);

  return TRUE;
}

static GHashTable *iso_3166 = NULL;
static GHashTable *iso_639 = NULL;

static gboolean
load_iso_data(void)
{
  if (!iso_3166)
  {
    iso_3166 = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    if (!parse_iso_3166(iso_3166))
    {
      g_hash_table_destroy(iso_3166);
      iso_3166 = NULL;
      return FALSE;
    }

    iso_639 = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    if (!parse_iso_639(iso_639))
    {
      g_hash_table_destroy(iso_3166);
      g_hash_table_destroy(iso_639);
      iso_3166 = NULL;
      iso_639 = NULL;
      return FALSE;
    }
  }

  return TRUE;
}

/**
 * @brief - Formats UI string from language_country identifer
 *
 * @param lang_id - identifier in form of language_COUNTRY, like en_US
 *
 * @return formatted string or NULL. You must free the result by using g_free()
 */
gchar *
iso_codes_locale_resolve_simple(const gchar *lang_id)
{
  gchar **id;
  gchar *lang;
  gchar *cntry;
  gchar *lang_env;
  gchar *old_locale;
  gchar *rv;

  g_return_val_if_fail(lang_id != NULL, NULL);

  if (!load_iso_data())
    return NULL;

  id = g_strsplit(lang_id, "_", -1);

  g_return_val_if_fail(g_strv_length(id) == 2, (g_strfreev(id), NULL));

  lang = g_hash_table_lookup(iso_639, id[0]);
  cntry = g_hash_table_lookup(iso_3166, id[1]);

  lang_env = getenv("LANGUAGE");

  if (lang_env)
    lang_env = g_strdup(lang_env);

  setenv("LANGUAGE", id[0], TRUE);
  old_locale = setlocale(LC_ALL, "");

  rv = g_strdup_printf("%s (%s)",
                       dgettext("iso_639_3", lang),
                       dgettext("iso_3166", cntry));

  if (lang_env)
  {
    setenv("LANGUAGE", lang_env, TRUE);
    g_free((lang_env));
  }
  else
    unsetenv("LANGUAGE");

  setlocale(LC_ALL, old_locale);

  g_strfreev(id);

  return rv;
}
