<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method = "xml" indent="yes" encoding="UTF-8" doctype-system = "tomoe-dict.dtd" standalone="no"/>
<xsl:template match="/">
  <tomoe_dictionary>
    <xsl:apply-templates select="//character"/>
  </tomoe_dictionary>
</xsl:template>
<xsl:template match="character">
  <character>
    <xsl:apply-templates select="literal|reading_meaning"/>
  </character>
</xsl:template>
<xsl:template match="literal">
  <literal>
    <xsl:value-of select="."/>
  </literal>
</xsl:template>
<xsl:template match="reading_meaning">
  <xsl:apply-templates select="rmgroup"/>
</xsl:template>
<xsl:template match="rmgroup">
  <readings>
    <xsl:apply-templates select="reading[@r_type='ja_on']|reading[@r_type='ja_kun']"/>
  </readings>
</xsl:template>
<xsl:template match="reading">
  <r>
    <xsl:value-of select="."/>
  </r>
</xsl:template>
</xsl:stylesheet>
